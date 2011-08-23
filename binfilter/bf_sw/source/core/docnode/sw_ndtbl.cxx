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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifdef WTC
#define private public
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFORDR_HXX //autogen
#include <fmtfordr.hxx>
#endif
#ifndef _FMTLSPLT_HXX
#include <fmtlsplt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _TBLAFMT_HXX
#include <tblafmt.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _TBLRWCL_HXX
#include <tblrwcl.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif

#ifndef _NODE2LAY_HXX
#include <node2lay.hxx>
#endif

#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif

#ifndef _DOCSH_HXX
#include "docsh.hxx"
#endif

#ifdef _MSAVE_HXX
#endif

#ifdef LINUX
#endif

#include <ndtxt.hxx>
namespace binfilter {

const sal_Unicode T2T_PARA = 0x0a;


// steht im gctable.cxx


/*N*/ void lcl_SetDfltBoxAttr( SwFrmFmt& rFmt, BYTE nId )
/*N*/ {
/*N*/ 	BOOL bTop = FALSE, bBottom = FALSE, bLeft = FALSE, bRight = FALSE;
/*N*/ 	switch ( nId )
/*N*/ 	{
/*?*/ 	case 0:	bTop = bBottom = bLeft = TRUE; 			break;
/*N*/ 	case 1:	bTop = bBottom = bLeft = bRight = TRUE;	break;
/*?*/ 	case 2:	bBottom = bLeft = TRUE; 				break;
/*?*/ 	case 3: bBottom = bLeft = bRight = TRUE; 		break;
/*N*/ 	}
/*N*/
/*N*/ 	const BOOL bHTML = rFmt.GetDoc()->IsHTMLMode();
/*N*/ 	Color aCol( bHTML ? COL_GRAY : COL_BLACK );
/*N*/ 	SvxBorderLine aLine( &aCol, DEF_LINE_WIDTH_0 );
/*N*/ 	if ( bHTML )
/*N*/ 	{
/*?*/ 		aLine.SetOutWidth( DEF_DOUBLE_LINE7_OUT );
/*?*/ 		aLine.SetInWidth ( DEF_DOUBLE_LINE7_IN  );
/*?*/ 		aLine.SetDistance( DEF_DOUBLE_LINE7_DIST);
/*N*/ 	}
/*N*/ 	SvxBoxItem aBox; aBox.SetDistance( 55 );
/*N*/ 	if ( bTop )
/*N*/ 		aBox.SetLine( &aLine, BOX_LINE_TOP );
/*N*/ 	if ( bBottom )
/*N*/ 		aBox.SetLine( &aLine, BOX_LINE_BOTTOM );
/*N*/ 	if ( bLeft )
/*N*/ 		aBox.SetLine( &aLine, BOX_LINE_LEFT );
/*N*/ 	if ( bRight )
/*N*/ 		aBox.SetLine( &aLine, BOX_LINE_RIGHT );
/*N*/ 	rFmt.SetAttr( aBox );
/*N*/ }


/* --> #109161# */
static bool lcl_IsItemSet(const SwCntntNode & rNode, USHORT which)
{
    bool bResult = false;

    if (SFX_ITEM_SET == rNode.GetSwAttrSet().GetItemState(which))
        bResult = true;

    return bResult;
}

/* <-- #109161# */

/*N*/ SwTableBoxFmt *lcl_CreateDfltBoxFmt( SwDoc &rDoc, SvPtrarr &rBoxFmtArr,
/*N*/ 									USHORT nCols, BYTE nId )
/*N*/ {
/*N*/ 	if ( !rBoxFmtArr[nId] )
/*N*/ 	{
/*N*/ 		SwTableBoxFmt* pBoxFmt = rDoc.MakeTableBoxFmt();
/*N*/ 		if( USHRT_MAX != nCols )
/*N*/ 			pBoxFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE,
/*N*/ 											USHRT_MAX / nCols, 0 ));
/*N*/ 		::binfilter::lcl_SetDfltBoxAttr( *pBoxFmt, nId );
/*N*/ 		rBoxFmtArr.Replace( pBoxFmt, nId );
/*N*/ 	}
/*N*/ 	return (SwTableBoxFmt*)rBoxFmtArr[nId];
/*N*/ }


/*N*/ SwTableNode* SwDoc::IsIdxInTbl(const SwNodeIndex& rIdx)
/*N*/ {
/*N*/ 	SwTableNode* pTableNd = 0;
/*N*/ 	ULONG nIndex = rIdx.GetIndex();
/*N*/ 	do {
/*N*/ 		SwNode* pNd = (SwNode*)GetNodes()[ nIndex ]->StartOfSectionNode();
/*N*/ 		if( 0 != ( pTableNd = pNd->GetTableNode() ) )
/*?*/ 			break;
/*N*/
/*N*/ 		nIndex = pNd->GetIndex();
/*N*/ 	} while ( nIndex );
/*N*/ 	return pTableNd;
/*N*/ }


// --------------- einfuegen einer neuen Box --------------

    // fuege in der Line, vor der InsPos eine neue Box ein.

 BOOL SwNodes::InsBoxen( SwTableNode* pTblNd,
                        SwTableLine* pLine,
                        SwTableBoxFmt* pBoxFmt,
                        SwTxtFmtColl* pTxtColl,
                        SwAttrSet* pAutoAttr,
                        USHORT nInsPos,
                        USHORT nCnt )
 {
    if( !nCnt )
        return FALSE;
    ASSERT( pLine, "keine gueltige Zeile" );

    // Index hinter die letzte Box der Line
    ULONG nIdxPos;
    SwTableBox *pPrvBox = 0, *pNxtBox = 0;
    if( pLine->GetTabBoxes().Count() )
    {
        if( nInsPos < pLine->GetTabBoxes().Count() )
        {
            if( 0 == (pPrvBox = pLine->FindPreviousBox( pTblNd->GetTable(),
                            pLine->GetTabBoxes()[ nInsPos ] )))
                pPrvBox = pLine->FindPreviousBox( pTblNd->GetTable() );
        }
        else if( 0 == ( pNxtBox = pLine->FindNextBox( pTblNd->GetTable(),
                            pLine->GetTabBoxes()[ nInsPos-1 ] )))
                pNxtBox = pLine->FindNextBox( pTblNd->GetTable() );
    }
    else if( 0 == ( pNxtBox = pLine->FindNextBox( pTblNd->GetTable() )))
        pPrvBox = pLine->FindPreviousBox( pTblNd->GetTable() );

    if( !pPrvBox && !pNxtBox )
    {
        BOOL bSetIdxPos = TRUE;
        if( pTblNd->GetTable().GetTabLines().Count() && !nInsPos )
        {
            const SwTableLine* pTblLn = pLine;
            while( pTblLn->GetUpper() )
                pTblLn = pTblLn->GetUpper()->GetUpper();

            if( pTblNd->GetTable().GetTabLines()[ 0 ] == pTblLn )
            {
                // also vor die erste Box der Tabelle
                while( ( pNxtBox = pLine->GetTabBoxes()[0])->GetTabLines().Count() )
                    pLine = pNxtBox->GetTabLines()[0];
                nIdxPos = pNxtBox->GetSttIdx();
                bSetIdxPos = FALSE;
            }
        }
        if( bSetIdxPos )
            // Tabelle ohne irgendeinen Inhalt oder am Ende, also vors Ende
            nIdxPos = pTblNd->EndOfSectionIndex();
    }
    else if( pNxtBox )          // es gibt einen Nachfolger
        nIdxPos = pNxtBox->GetSttIdx();
    else                        // es gibt einen Vorgaenger
        nIdxPos = pPrvBox->GetSttNd()->EndOfSectionIndex() + 1;

    SwNodeIndex aEndIdx( *this, nIdxPos );
    for( USHORT n = 0; n < nCnt; ++n )
    {
        SwStartNode* pSttNd = new SwStartNode( aEndIdx, ND_STARTNODE,
                                                SwTableBoxStartNode );
        pSttNd->pStartOfSection = pTblNd;
        SwEndNode* pEndNd = new SwEndNode( aEndIdx, *pSttNd );

        pPrvBox = new SwTableBox( pBoxFmt, *pSttNd, pLine );
        pLine->GetTabBoxes().C40_INSERT( SwTableBox, pPrvBox, nInsPos + n );

        if( NO_NUMBERING == pTxtColl->GetOutlineLevel()
 //FEATURE::CONDCOLL
            && RES_CONDTXTFMTCOLL != pTxtColl->Which()
 //FEATURE::CONDCOLL
        )
            new SwTxtNode( SwNodeIndex( *pSttNd->EndOfSectionNode() ),
                                pTxtColl, pAutoAttr );
        else
        {
            // Outline-Numerierung richtig behandeln !!!
            SwTxtNode* pTNd = new SwTxtNode(
                            SwNodeIndex( *pSttNd->EndOfSectionNode() ),
                            (SwTxtFmtColl*)GetDoc()->GetDfltTxtFmtColl(),
                            pAutoAttr );
            pTNd->ChgFmtColl( pTxtColl );
        }
    }
    return TRUE;
 }

// --------------- einfuegen einer neuen Tabelle --------------

/*N*/ const SwTable* SwDoc::InsertTable( const SwPosition& rPos, USHORT nRows,
/*N*/ 									USHORT nCols, SwHoriOrient eAdjust,
/*N*/ 									USHORT nInsTblFlags,
/*N*/ 									const SwTableAutoFmt* pTAFmt,
/*N*/                                    const SvUShorts* pColArr,
/*N*/                                    BOOL bCalledFromShell )
/*N*/ {
/*N*/ 	ASSERT( nRows, "Tabelle ohne Zeile?" );
/*N*/ 	ASSERT( nCols, "Tabelle ohne Spalten?" );
/*N*/
/*N*/ 	{
/*N*/ 		// nicht in Fussnoten kopieren !!
/*N*/ 		if( rPos.nNode < GetNodes().GetEndOfInserts().GetIndex() &&
/*N*/ 			rPos.nNode >= GetNodes().GetEndOfInserts().StartOfSectionIndex() )
/*?*/ 			return 0;
/*N*/
/*N*/ 		// sollte das ColumnArray die falsche Anzahl haben wird es ignoriert!
/*N*/ 		if( pColArr &&
/*N*/ 			(nCols + ( HORI_NONE == eAdjust ? 2 : 1 )) != pColArr->Count() )
/*?*/ 			pColArr = 0;
/*N*/ 	}
/*N*/
/*N*/ 	// fuege erstmal die Nodes ein
/*N*/ 	// hole das Auto-Format fuer die Tabelle
/*N*/ 	SwTxtFmtColl *pBodyColl = GetTxtCollFromPool( RES_POOLCOLL_TABLE ),
/*N*/ 				 *pHeadColl = pBodyColl;
/*N*/
/*N*/ 	BOOL bDfltBorders = nInsTblFlags & DEFAULT_BORDER;
/*N*/
/*N*/ 	if( (nInsTblFlags & HEADLINE) && (1 != nRows || !bDfltBorders) )
/*?*/ 		pHeadColl = GetTxtCollFromPool( RES_POOLCOLL_TABLE_HDLN );
/*N*/
/*N*/ 	/* #106283# Save content node to extract FRAMEDIR from. */
/*N*/ 	const SwCntntNode * pCntntNd = rPos.nNode.GetNode().GetCntntNode();
/*N*/
    /* #109161# If we are called from a shell pass the attrset from
        pCntntNd (aka the node the table is inserted at) thus causing
        SwNodes::InsertTable to propagate an adjust item if
        necessary. */
/*N*/ 	SwTableNode *pTblNd = GetNodes().InsertTable
/*N*/         ( rPos.nNode, nCols, pBodyColl, nRows, pHeadColl,
/*N*/           bCalledFromShell ? &pCntntNd->GetSwAttrSet() : 0 );
/*N*/
/*N*/ 	// dann erstelle die Box/Line/Table-Struktur
/*N*/ 	SwTableLineFmt* pLineFmt = MakeTableLineFmt();
/*N*/ 	SwTableFmt* pTableFmt = MakeTblFrmFmt( GetUniqueTblName(), GetDfltFrmFmt() );
/*N*/
    /* #106283# If the node to insert the table at is a context node and has a
       non-default FRAMEDIR propagate it to the table. */
/*N*/ 	if (pCntntNd)
/*N*/ 	{
/*N*/ 		const SwAttrSet & aNdSet = pCntntNd->GetSwAttrSet();
/*N*/ 		const SfxPoolItem *pItem = NULL;
/*N*/
/*N*/ 		if (SFX_ITEM_SET == aNdSet.GetItemState( RES_FRAMEDIR, TRUE, &pItem )
/*N*/ 			&& pItem != NULL)
/*N*/ 		{
/*N*/ 			pTableFmt->SetAttr( *pItem );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	//Orientation am Fmt der Table setzen
/*N*/ 	pTableFmt->SetAttr( SwFmtHoriOrient( 0, eAdjust ) );
/*N*/ 	// alle Zeilen haben die Fill-Order von links nach rechts !
/*N*/ 	pLineFmt->SetAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ));
/*N*/
/*N*/ 	// die Tabelle bekommt USHRT_MAX als default SSize
/*N*/ 	SwTwips nWidth = USHRT_MAX;
/*N*/ 	if( pColArr )
/*N*/ 	{
/*?*/ 		USHORT nSttPos = (*pColArr)[ 0 ];
/*?*/ 		USHORT nLastPos = (*pColArr)[ USHORT(pColArr->Count()-1)];
/*?*/ 		if( HORI_NONE == eAdjust )
/*?*/ 		{
/*?*/ 			USHORT nFrmWidth = nLastPos;
/*?*/ 			nLastPos = (*pColArr)[ USHORT(pColArr->Count()-2)];
/*?*/ 			pTableFmt->SetAttr( SvxLRSpaceItem( nSttPos, nFrmWidth - nLastPos ) );
/*?*/ 		}
/*?*/ 		nWidth = nLastPos - nSttPos;
/*N*/ 	}
/*N*/ 	pTableFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth ));
/*N*/ 	if( !(nInsTblFlags & SPLIT_LAYOUT) )
/*?*/ 		pTableFmt->SetAttr( SwFmtLayoutSplit( FALSE ));
/*N*/
/*N*/ 	// verschiebe ggfs. die harten PageDesc/PageBreak Attribute:
/*N*/ 	SwCntntNode* pNextNd = GetNodes()[ pTblNd->EndOfSectionIndex()+1 ]
/*N*/ 							->GetCntntNode();
/*N*/ 	if( pNextNd && pNextNd->GetpSwAttrSet() )
/*N*/ 	{
/*?*/ 		SwAttrSet* pNdSet = pNextNd->GetpSwAttrSet();
/*?*/ 		const SfxPoolItem *pItem;
/*?*/ 		if( SFX_ITEM_SET == pNdSet->GetItemState( RES_PAGEDESC, FALSE,
/*?*/ 			&pItem ) )
/*?*/ 		{
/*?*/ 			pTableFmt->SetAttr( *pItem );
/*?*/ 			pNextNd->ResetAttr( RES_PAGEDESC );
/*?*/ 			pNdSet = pNextNd->GetpSwAttrSet();
/*?*/ 		}
/*?*/ 		if( pNdSet && SFX_ITEM_SET == pNdSet->GetItemState( RES_BREAK, FALSE,
/*?*/ 			 &pItem ) )
/*?*/ 		{
/*?*/ 			pTableFmt->SetAttr( *pItem );
/*?*/ 			pNextNd->ResetAttr( RES_BREAK );
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	SwTable * pNdTbl = &pTblNd->GetTable();
/*N*/ 	pTableFmt->Add( pNdTbl );		// das Frame-Format setzen
/*N*/
/*N*/ 	pNdTbl->SetHeadlineRepeat( HEADLINE_REPEAT == (nInsTblFlags & HEADLINE_REPEAT) );
/*N*/
/*N*/ 	SvPtrarr aBoxFmtArr( 0, 16 );
/*N*/ 	SwTableBoxFmt* pBoxFmt = 0;
/*N*/ 	if( !bDfltBorders && !pTAFmt )
/*N*/ 	{
/*?*/ 		pBoxFmt = MakeTableBoxFmt();
/*?*/ 		pBoxFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, USHRT_MAX / nCols, 0 ));
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const USHORT nBoxArrLen = pTAFmt ? 16 : 4;
/*N*/ 		for( USHORT i = 0; i < nBoxArrLen; ++i )
/*N*/ 			aBoxFmtArr.Insert( (void*)0, i );
/*N*/ 	}
/*N*/ 	SfxItemSet aCharSet( GetAttrPool(), RES_CHRATR_BEGIN, RES_PARATR_END-1 );
/*N*/
/*N*/ 	SwNodeIndex aNdIdx( *pTblNd, 1 );	// auf den ersten Box-StartNode
/*N*/ 	SwTableLines& rLines = pNdTbl->GetTabLines();
/*N*/ 	for( USHORT n = 0; n < nRows; ++n )
/*N*/ 	{
/*N*/ 		SwTableLine* pLine = new SwTableLine( pLineFmt, nCols, 0 );
/*N*/ 		rLines.C40_INSERT( SwTableLine, pLine, n );
/*N*/ 		SwTableBoxes& rBoxes = pLine->GetTabBoxes();
/*N*/ 		for( USHORT i = 0; i < nCols; ++i )
/*N*/ 		{
/*N*/ 			SwTableBoxFmt *pBoxF;
/*N*/ 			if( pTAFmt )
/*N*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 BYTE nId = !n ? 0 : (( n+1 == nRows )
/*N*/ 			}
/*N*/ 			else if( bDfltBorders )
/*N*/ 			{
/*N*/ 				BYTE nBoxId = (i < nCols - 1 ? 0 : 1) + (n ? 2 : 0 );
/*N*/ 				pBoxF = ::binfilter::lcl_CreateDfltBoxFmt( *this, aBoxFmtArr, nCols, nBoxId);
/*N*/ 			}
/*N*/ 			else
/*?*/ 				pBoxF = pBoxFmt;
/*N*/
/*N*/ 			// fuer AutoFormat bei der Eingabe: beim Einfuegen der Tabelle
/*N*/ 			// werden gleich die Spalten gesetzt. Im Array stehen die
/*N*/ 			// Positionen der Spalten!! (nicht deren Breite!)
/*N*/ 			if( pColArr )
/*N*/ 			{
/*?*/ 				nWidth = (*pColArr)[ USHORT(i + 1) ] - (*pColArr)[ i ];
/*?*/ 				if( pBoxF->GetFrmSize().GetWidth() != nWidth )
/*?*/ 				{
/*?*/ 					if( pBoxF->GetDepends() )		// neues Format erzeugen!
/*?*/ 					{
/*?*/ 						SwTableBoxFmt *pNewFmt = MakeTableBoxFmt();
/*?*/ 						*pNewFmt = *pBoxF;
/*?*/ 						pBoxF = pNewFmt;
/*?*/ 					}
/*?*/ 					pBoxF->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth ));
/*?*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 			SwTableBox *pBox = new SwTableBox( pBoxF, aNdIdx, pLine);
/*N*/ 			rBoxes.C40_INSERT( SwTableBox, pBox, i );
/*N*/ 			aNdIdx += 3;		// StartNode, TextNode, EndNode  == 3 Nodes
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// und Frms einfuegen.
/*N*/ 	GetNodes().GoNext( &aNdIdx );      // zum naechsten ContentNode
/*N*/ 	pTblNd->MakeFrms( &aNdIdx );
/*N*/
/*N*/ 	if( IsRedlineOn() || (!IsIgnoreRedline() && pRedlineTbl->Count() ))
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwPaM aPam( *pTblNd->EndOfSectionNode(), *pTblNd, 1 );
/*N*/ 	}
/*N*/
/*N*/ 	SetModified();
/*N*/ 	return pNdTbl;
/*N*/ }

/*N*/ SwTableNode* SwNodes::InsertTable( const SwNodeIndex& rNdIdx,
/*N*/ 									USHORT nBoxes,
/*N*/ 									SwTxtFmtColl* pCntntTxtColl,
/*N*/ 									USHORT nLines,
/*N*/ 								   SwTxtFmtColl* pHeadlineTxtColl,
/*N*/                                    const SwAttrSet * pAttrSet)
/*N*/ {
/*N*/ 	if( !nBoxes )
/*?*/ 		return 0;
/*N*/
/*N*/ 	// wenn Lines angegeben, erzeuge die Matrix aus Lines & Boxen
/*N*/ 	if( !pHeadlineTxtColl || !nLines )
/*N*/ 		pHeadlineTxtColl = pCntntTxtColl;
/*N*/
/*N*/ 	SwTableNode * pTblNd = new SwTableNode( rNdIdx );
/*N*/ 	SwEndNode* pEndNd = new SwEndNode( rNdIdx, *pTblNd );
/*N*/
/*N*/ 	if( !nLines )		// fuer die FOR-Schleife
/*N*/ 		++nLines;
/*N*/
/*N*/ 	SwNodeIndex aIdx( *pEndNd );
/*N*/ 	register SwTxtFmtColl* pTxtColl = pHeadlineTxtColl;
/*N*/ 	for( register USHORT nL = 0; nL < nLines; ++nL )
/*N*/ 	{
/*N*/ 		for( register USHORT nB = 0; nB < nBoxes; ++nB )
/*N*/ 		{
/*N*/ 			SwStartNode* pSttNd = new SwStartNode( aIdx, ND_STARTNODE,
/*N*/ 													SwTableBoxStartNode );
/*N*/ 			pSttNd->pStartOfSection = pTblNd;

            /** #109161# If there is no adjust item in pTxtColl
                 propagate any existing adjust item in pAttrSet to the
                 newly created context node in the new cell.
             */
/*N*/ 			SwTxtNode * pTmpNd = new SwTxtNode( aIdx, pTxtColl );
/*N*/
/*N*/             const SfxPoolItem * pItem = NULL;
/*N*/
/*N*/             if (! lcl_IsItemSet(*pTmpNd, RES_PARATR_ADJUST) &&
/*N*/                 pAttrSet != NULL &&
/*N*/                 SFX_ITEM_SET == pAttrSet->GetItemState( RES_PARATR_ADJUST, TRUE,
/*N*/                                                         &pItem)
/*N*/                 )
/*N*/             {
/*N*/                 static_cast<SwCntntNode *>(pTmpNd)->SetAttr(*pItem);
/*N*/             }
/*N*/
/*N*/ 			new SwEndNode( aIdx, *pSttNd );
/*N*/ 		}
/*N*/ 		pTxtColl = pCntntTxtColl;
/*N*/ 	}
/*N*/ 	return pTblNd;
/*N*/ }


//---------------- Text -> Tabelle -----------------------



//---------------- Tabelle -> Text -----------------------



// -- benutze die ForEach Methode vom PtrArray um aus einer Tabelle wieder
//		Text zuerzeugen. (Die Boxen koennen auch noch Lines enthalten !!)

// forward deklarieren damit sich die Lines und Boxen rekursiv aufrufen
// koennen.







// ----- einfuegen von Spalten/Zeilen ------------------------





// ----- loeschen von Spalten/Zeilen ------------------------





// ---------- teilen / zusammenfassen von Boxen in der Tabelle --------

/*N*/ BOOL SwDoc::SplitTbl( const SwSelBoxes& rBoxes, sal_Bool bVert, USHORT nCnt,
/*N*/                       sal_Bool bSameHeight )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001
/*N*/ }





// -------------------------------------------------------

//---------
// SwTableNode
//---------

/*N*/ SwTableNode::SwTableNode( const SwNodeIndex& rIdx )
/*N*/ 	: SwStartNode( rIdx, ND_TABLENODE )
/*N*/ {
/*N*/ 	pTable = new SwTable( 0 );
/*N*/ }

/*M*/ SwTableNode::~SwTableNode()
/*M*/ {
/*M*/ 	//don't forget to notify uno wrappers
/*M*/ 	SwFrmFmt* pTblFmt = GetTable().GetFrmFmt();
/*M*/ 	SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT,
/*M*/ 								pTblFmt );
/*M*/ 	pTblFmt->Modify( &aMsgHint, &aMsgHint );
/*M*/ 	DelFrms();
/*M*/ 	delete pTable;
/*M*/ }

/*N*/ SwTabFrm *SwTableNode::MakeFrm()
/*N*/ {
/*N*/ 	return new SwTabFrm( *pTable );
/*N*/ }

//Fuer jede Shell einen TblFrm anlegen und vor den entsprechenden
//CntntFrm pasten.

/*N*/ void SwTableNode::MakeFrms( SwNodeIndex* pIdxBehind )
/*N*/ {
/*N*/ 	ASSERT( pIdxBehind, "kein Index" );
/*N*/ 	*pIdxBehind = *this;
/*N*/ 	SwNode *pNd = GetNodes().FindPrvNxtFrmNode( *pIdxBehind, EndOfSectionNode() );
/*N*/ 	if( !pNd )
/*N*/ 		return ;
/*N*/
/*N*/ 	// liegt der gefundene ContentNode vor oder hinter der Tabelle ?
/*N*/ 	BOOL bBehind = EndOfSectionIndex() < pIdxBehind->GetIndex();
/*N*/
/*N*/ 	SwFrm *pFrm, *pNew;
/*N*/
/*N*/ 	SwNode2Layout aNode2Layout( *pNd, GetIndex() );
/*N*/ 	while( 0 != (pFrm = aNode2Layout.NextFrm()) )
/*N*/ 	{
/*N*/ 		pNew = MakeFrm();
/*N*/ 		pNew->Paste( pFrm->GetUpper(),	bBehind ? pFrm : pFrm->GetNext() );
/*N*/ 		((SwTabFrm*)pNew)->RegistFlys();
/*N*/ 	}
/*N*/ }

/*N*/ void SwTableNode::DelFrms()
/*N*/ {
/*N*/ 	//Erstmal die TabFrms ausschneiden und deleten, die Columns und Rows
/*N*/ 	//nehmen sie mit in's Grab.
/*N*/ 	//Die TabFrms haengen am FrmFmt des SwTable.
/*N*/ 	//Sie muessen etwas umstaendlich zerstort werden, damit die Master
/*N*/ 	//die Follows mit in's Grab nehmen.
/*N*/
/*N*/ 	SwClientIter aIter( *(pTable->GetFrmFmt()) );
/*N*/ 	SwClient *pLast = aIter.GoStart();
/*N*/ 	while ( pLast )
/*N*/ 	{
/*N*/ 		BOOL bAgain = FALSE;
/*N*/ 		if ( pLast->IsA( TYPE(SwFrm) ) )
/*N*/ 		{
/*N*/ 			SwTabFrm *pFrm = (SwTabFrm*)pLast;
/*N*/ 			if ( !pFrm->IsFollow() )
/*N*/ 			{
/*N*/ 				while ( pFrm->HasFollow() )
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	pFrm->JoinAndDelFollows();
/*N*/ 				pFrm->Cut();
/*N*/ 				delete pFrm;
/*N*/ 				bAgain = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pLast = bAgain ? aIter.GoStart() : aIter++;
/*N*/ 	}
/*N*/ }


/*N*/ void SwTableNode::SetNewTable( SwTable* pNewTable, BOOL bNewFrames )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	DelFrms();
/*N*/ }

    // setze das TabelleAttribut Undo auf:



/* -----------------18.07.98 11:45-------------------
 *  Direktzugriff fuer UNO
 * --------------------------------------------------*/

void SwCollectTblLineBoxes::AddBox( const SwTableBox& rBox )
{
    aPosArr.Insert( nWidth, aPosArr.Count() );
    SwTableBox* p = (SwTableBox*)&rBox;
    aBoxes.Insert( p, aBoxes.Count() );
    nWidth += (USHORT)rBox.GetFrmFmt()->GetFrmSize().GetWidth();
}

const SwTableBox* SwCollectTblLineBoxes::GetBoxOfPos( const SwTableBox& rBox )
{
    const SwTableBox* pRet = 0;
    if( aPosArr.Count() )
    {
        USHORT n;
        for( n = 0; n < aPosArr.Count(); ++n )
            if( aPosArr[ n ] == nWidth )
                break;
            else if( aPosArr[ n ] > nWidth )
            {
                if( n )
                    --n;
                break;
            }

        if( n >= aPosArr.Count() )
            --n;

        nWidth += (USHORT)rBox.GetFrmFmt()->GetFrmSize().GetWidth();
        pRet = aBoxes[ n ];
    }
    return pRet;
}

BOOL lcl_Line_CollectBox( const SwTableLine*& rpLine, void* pPara )
{
    SwCollectTblLineBoxes* pSplPara = (SwCollectTblLineBoxes*)pPara;
    if( pSplPara->IsGetValues() )
        ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &::binfilter::lcl_Box_CollectBox, pPara );
    else
        ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &::binfilter::lcl_BoxSetSplitBoxFmts, pPara );
    return TRUE;
}

BOOL lcl_Box_CollectBox( const SwTableBox*& rpBox, void* pPara )
{
    SwCollectTblLineBoxes* pSplPara = (SwCollectTblLineBoxes*)pPara;
    USHORT nLen = rpBox->GetTabLines().Count();
    if( nLen )
    {
        // dann mit der richtigen Line weitermachen
        if( pSplPara->IsGetFromTop() )
            nLen = 0;
        else
            --nLen;

        const SwTableLine* pLn = rpBox->GetTabLines()[ nLen ];
        lcl_Line_CollectBox( pLn, pPara );
    }
    else
        pSplPara->AddBox( *rpBox );
    return TRUE;
}

BOOL lcl_BoxSetSplitBoxFmts( const SwTableBox*& rpBox, void* pPara )
{
    SwCollectTblLineBoxes* pSplPara = (SwCollectTblLineBoxes*)pPara;
    USHORT nLen = rpBox->GetTabLines().Count();
    if( nLen )
    {
        // dann mit der richtigen Line weitermachen
        if( pSplPara->IsGetFromTop() )
            nLen = 0;
        else
            --nLen;

        const SwTableLine* pLn = rpBox->GetTabLines()[ nLen ];
        lcl_Line_CollectBox( pLn, pPara );
    }
    else
    {
        const SwTableBox* pSrcBox = pSplPara->GetBoxOfPos( *rpBox );
        SwFrmFmt* pFmt = pSrcBox->GetFrmFmt();
        SwTableBox* pBox = (SwTableBox*)rpBox;

        if( HEADLINE_BORDERCOPY == pSplPara->GetMode() )
        {
            const SvxBoxItem& rBoxItem = pBox->GetFrmFmt()->GetBox();
            if( !rBoxItem.GetTop() )
            {
                SvxBoxItem aNew( rBoxItem );
                aNew.SetLine( pFmt->GetBox().GetBottom(), BOX_LINE_TOP );
                if( aNew != rBoxItem )
                    pBox->ClaimFrmFmt()->SetAttr( aNew );
            }
        }
        else
        {
USHORT __FAR_DATA aTableSplitBoxSetRange[] = {
    RES_LR_SPACE,       RES_UL_SPACE,
    RES_BACKGROUND,     RES_SHADOW,
    RES_PROTECT,        RES_PROTECT,
    RES_VERT_ORIENT,    RES_VERT_ORIENT,
    0 };
            SfxItemSet aTmpSet( pFmt->GetDoc()->GetAttrPool(),
                                aTableSplitBoxSetRange );
            aTmpSet.Put( pFmt->GetAttrSet() );
            if( aTmpSet.Count() )
                pBox->ClaimFrmFmt()->SetAttr( aTmpSet );

            if( HEADLINE_BOXATRCOLLCOPY == pSplPara->GetMode() )
            {
                SwNodeIndex aIdx( *pSrcBox->GetSttNd(), 1 );
                SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
                if( !pCNd )
                    pCNd = aIdx.GetNodes().GoNext( &aIdx );
                aIdx = *pBox->GetSttNd();
                SwCntntNode* pDNd = aIdx.GetNodes().GoNext( &aIdx );

                // nur wenn der Node alleine in der Section steht
                if( 2 == pDNd->EndOfSectionIndex() -
                        pDNd->StartOfSectionIndex() )
                {
                    pDNd->ChgFmtColl( pCNd->GetFmtColl() );
                }
            }

            // bedingte Vorlage beachten
            pBox->GetSttNd()->CheckSectionCondColl();
        }
    }
    return TRUE;
}



// und die Umkehrung davon. rPos muss in der Tabelle stehen, die bestehen
// bleibt. Das Flag besagt ob die aktuelle mit der davor oder dahinter
// stehenden vereint wird.


// -------------------------------------------------------------------


// -- benutze die ForEach Methode vom PtrArray

// forward deklarieren damit sich die Lines und Boxen rekursiv aufrufen
// koennen.




        // AutoFormat fuer die Tabelle/TabellenSelection


        // Erfrage wie attributiert ist

/*N*/ String SwDoc::GetUniqueTblName() const
/*N*/ {
/*N*/ 	ResId aId( STR_TABLE_DEFNAME, *pSwResMgr );
/*N*/ 	String aName( aId );
/*N*/ 	xub_StrLen nNmLen = aName.Len();
/*N*/
/*N*/ 	USHORT nNum, nTmp, nFlagSize = ( pTblFrmFmtTbl->Count() / 8 ) +2;
/*N*/ 	BYTE* pSetFlags = new BYTE[ nFlagSize ];
/*N*/ 	memset( pSetFlags, 0, nFlagSize );
/*N*/
        USHORT n=0;
/*N*/ 	for( n = 0; n < pTblFrmFmtTbl->Count(); ++n )
/*N*/ 	{
/*N*/ 		const SwFrmFmt* pFmt = (*pTblFrmFmtTbl)[ n ];
/*N*/ 		if( !pFmt->IsDefault() && IsUsed( *pFmt )  &&
/*N*/ 			pFmt->GetName().Match( aName ) == nNmLen )
/*N*/ 		{
/*N*/ 			// Nummer bestimmen und das Flag setzen
/*N*/ 			nNum = pFmt->GetName().Copy( nNmLen ).ToInt32();
/*N*/ 			if( nNum-- && nNum < pTblFrmFmtTbl->Count() )
/*N*/ 				pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	// alle Nummern entsprechend geflag, also bestimme die richtige Nummer
/*N*/ 	nNum = pTblFrmFmtTbl->Count();
/*N*/ 	for( n = 0; n < nFlagSize; ++n )
/*N*/ 		if( 0xff != ( nTmp = pSetFlags[ n ] ))
/*N*/ 		{
/*N*/ 			// also die Nummer bestimmen
/*N*/ 			nNum = n * 8;
/*N*/ 			while( nTmp & 1 )
/*N*/ 				++nNum, nTmp >>= 1;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 	delete [] pSetFlags;
/*N*/ 	return aName += String::CreateFromInt32( ++nNum );
/*N*/ }

/*N*/ SwTableFmt* SwDoc::FindTblFmtByName( const String& rName, BOOL bAll ) const
/*N*/ {
/*N*/ 	const SwFmt* pRet = 0;
/*N*/ 	if( bAll )
/*?*/ 		pRet = FindFmtByName( (SvPtrarr&)*pTblFrmFmtTbl, rName );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// dann nur die, die im Doc gesetzt sind
/*N*/ 		for( USHORT n = 0; n < pTblFrmFmtTbl->Count(); ++n )
/*N*/ 		{
/*N*/ 			const SwFrmFmt* pFmt = (*pTblFrmFmtTbl)[ n ];
/*N*/ 			if( !pFmt->IsDefault() && IsUsed( *pFmt ) &&
/*N*/ 				pFmt->GetName() == rName )
/*N*/ 			{
/*?*/ 				pRet = pFmt;
/*?*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return (SwTableFmt*)pRet;
/*N*/ }





/*N*/ void SwDoc::ClearBoxNumAttrs( const SwNodeIndex& rNode )
/*N*/ {
/*N*/ 	SwStartNode* pSttNd;
/*N*/ 	if( 0 != ( pSttNd = GetNodes()[ rNode ]->
/*N*/ 								FindSttNodeByType( SwTableBoxStartNode )) &&
/*N*/ 		2 == pSttNd->EndOfSectionIndex() - pSttNd->GetIndex() )
/*N*/ 	{
/*N*/ 		SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().
/*N*/ 							GetTblBox( pSttNd->GetIndex() );
/*N*/
/*N*/ 		const SfxPoolItem* pFmtItem = 0;
/*N*/ 		const SfxItemSet& rSet = pBox->GetFrmFmt()->GetAttrSet();
/*N*/ 		if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMAT, FALSE, &pFmtItem ) ||
/*N*/ 			SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMULA, FALSE ) ||
/*N*/ 			SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_VALUE, FALSE ))
/*N*/ 		{
/*?*/ 			SwFrmFmt* pBoxFmt = pBox->ClaimFrmFmt();
/*?*/
/*?*/ 			//JP 01.09.97: TextFormate bleiben erhalten!
/*?*/ 			USHORT nWhich1 = RES_BOXATR_FORMAT;
/*?*/ 			if( pFmtItem && GetNumberFormatter()->IsTextFormat(
/*?*/ 					((SwTblBoxNumFormat*)pFmtItem)->GetValue() ))
/*?*/ 				nWhich1 = RES_BOXATR_FORMULA;
/*?*/ 			else
/*?*/ 				// JP 15.01.99: Nur Attribute zuruecksetzen reicht nicht.
/*?*/ 				//				Sorge dafuer, das der Text auch entsprechend
/*?*/ 				//				formatiert wird!
/*?*/ 				pBoxFmt->SetAttr( *GetDfltAttr( RES_BOXATR_FORMAT ));
/*?*/
/*?*/ 			pBoxFmt->ResetAttr( nWhich1, RES_BOXATR_VALUE );
/*?*/ 			SetModified();
/*N*/ 		}
/*N*/ 	}
/*N*/ }



// kopiert eine Tabelle aus dem selben oder einem anderen Doc in sich
// selbst. Dabei wird eine neue Tabelle angelegt oder eine bestehende
// mit dem Inhalt gefuellt; wobei entweder der Inhalt ab einer Box oder
// in eine bestehende TblSelektion gefuellt wird.
// Gerufen wird es von: edglss.cxx/fecopy.cxx













}
