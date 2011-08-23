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
// INCLUDE ---------------------------------------------------------------

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_sfx2/objsh.hxx>
#include <bf_svtools/zforlist.hxx>

#include "scitems.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "attarray.hxx"
#include "cellform.hxx"
#include "detfunc.hxx"			// fuer Notizen bei DeleteRange
namespace binfilter {

// Err527 Workaround
extern const ScFormulaCell* pLastFormulaTreeTop;	// in cellform.cxx

// STATIC DATA -----------------------------------------------------------

/*N*/ BOOL ScColumn::bDoubleAlloc = FALSE;	// fuer Import: Groesse beim Allozieren verdoppeln
/*N*/ 
/*N*/ 
/*N*/ void ScColumn::Insert( USHORT nRow, ScBaseCell* pNewCell )
/*N*/ {
/*N*/ 	BOOL bIsAppended = FALSE;
/*N*/ 	if (pItems && nCount)
/*N*/ 	{
/*N*/ 		if (pItems[nCount-1].nRow < nRow)
/*N*/ 		{
/*N*/ 			Append(nRow, pNewCell );
/*N*/ 			bIsAppended = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( !bIsAppended )
/*N*/ 	{
/*N*/ 		USHORT	nIndex;
/*N*/ 		if (Search(nRow, nIndex))
/*N*/ 		{
/*N*/ 			ScBaseCell* pOldCell = pItems[nIndex].pCell;
/*N*/ 			ScBroadcasterList* pBC = pOldCell->GetBroadcaster();
/*N*/ 			if (pBC && !pNewCell->GetBroadcaster())
/*N*/ 			{
/*N*/ 				pNewCell->SetBroadcaster( pBC );
/*N*/ 				pOldCell->ForgetBroadcaster();
/*N*/ 			}
/*N*/ 			if (pOldCell->GetNotePtr() && !pNewCell->GetNotePtr())
/*?*/ 				pNewCell->SetNote( *pOldCell->GetNotePtr() );
/*N*/ 			if ( pOldCell->GetCellType() == CELLTYPE_FORMULA && !pDocument->IsClipOrUndo() )
/*N*/ 			{
/*?*/ 				pOldCell->EndListeningTo( pDocument );
/*?*/ 				// falls in EndListening NoteCell in gleicher Col zerstoert
/*?*/ 				if ( nIndex >= nCount || pItems[nIndex].nRow != nRow )
/*?*/ 					Search(nRow, nIndex);
/*N*/ 			}
/*N*/ 			pOldCell->Delete();
/*N*/ 			pItems[nIndex].pCell = pNewCell;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if (nCount + 1 > nLimit)
/*N*/ 			{
/*N*/ 				if (bDoubleAlloc)
/*N*/ 				{
/*N*/ 					if (nLimit < COLUMN_DELTA)
/*N*/ 						nLimit = COLUMN_DELTA;
/*N*/ 					else
/*N*/ 					{
/*N*/ 						nLimit *= 2;
/*N*/ 						if ( nLimit > MAXROW+1 )
/*N*/ 							nLimit = MAXROW+1;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					nLimit += COLUMN_DELTA;
/*N*/ 
/*N*/ 				ColEntry* pNewItems = new ColEntry[nLimit];
/*N*/ 				if (pItems)
/*N*/ 				{
/*N*/ 					memmove( pNewItems, pItems, nCount * sizeof(ColEntry) );
/*N*/ 					delete[] pItems;
/*N*/ 				}
/*N*/ 				pItems = pNewItems;
/*N*/ 			}
/*N*/ 			memmove( &pItems[nIndex + 1], &pItems[nIndex], (nCount - nIndex) * sizeof(ColEntry) );
/*N*/ 			pItems[nIndex].pCell = pNewCell;
/*N*/ 			pItems[nIndex].nRow  = nRow;
/*N*/ 			++nCount;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Bei aus Clipboard sind hier noch falsche (alte) Referenzen!
/*N*/ 	// Werden in CopyBlockFromClip per UpdateReference umgesetzt,
/*N*/ 	// danach StartListeningFromClip und BroadcastFromClip gerufen.
/*N*/ 	// Wird ins Clipboard/UndoDoc gestellt, wird kein Broadcast gebraucht.
/*N*/ 	// Nach Import wird CalcAfterLoad gerufen, dort Listening.
/*N*/ 	if ( !(pDocument->IsClipOrUndo() || pDocument->IsInsertingFromOtherDoc()) )
/*N*/ 	{
/*N*/ 		pNewCell->StartListeningTo( pDocument );
/*N*/ 		CellType eCellType = pNewCell->GetCellType();
/*N*/ 		// Notizzelle entsteht beim Laden nur durch StartListeningCell,
/*N*/ 		// ausloesende Formelzelle muss sowieso dirty sein.
/*N*/ 		if ( !(pDocument->IsCalcingAfterLoad() && eCellType == CELLTYPE_NOTE) )
/*N*/ 		{
/*N*/ 			if ( eCellType == CELLTYPE_FORMULA )
/*N*/ 				((ScFormulaCell*)pNewCell)->SetDirty();
/*N*/ 			else
/*N*/ 				pDocument->Broadcast( ScHint( SC_HINT_DATACHANGED,
/*N*/ 					ScAddress( nCol, nRow, nTab ), pNewCell ) );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScColumn::Insert( USHORT nRow, ULONG nNumberFormat, ScBaseCell* pCell )
/*N*/ {
/*N*/ 	Insert(nRow, pCell);
/*N*/ 	short eOldType = pDocument->GetFormatTable()->
/*N*/ 						GetType( (ULONG)
/*N*/ 							((SfxUInt32Item*)GetAttr( nRow, ATTR_VALUE_FORMAT ))->
/*N*/ 								GetValue() );
/*N*/ 	short eNewType = pDocument->GetFormatTable()->GetType(nNumberFormat);
/*N*/ 	if (!pDocument->GetFormatTable()->IsCompatible(eOldType, eNewType))
/*N*/ 		ApplyAttr( nRow, SfxUInt32Item( ATTR_VALUE_FORMAT, (UINT32) nNumberFormat) );
/*N*/ }


/*N*/ void ScColumn::Append( USHORT nRow, ScBaseCell* pCell )
/*N*/ {
/*N*/ 	if (nCount + 1 > nLimit)
/*N*/ 	{
/*N*/ 		if (bDoubleAlloc)
/*N*/ 		{
/*N*/ 			if (nLimit < COLUMN_DELTA)
/*N*/ 				nLimit = COLUMN_DELTA;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				nLimit *= 2;
/*N*/ 				if ( nLimit > MAXROW+1 )
/*N*/ 					nLimit = MAXROW+1;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nLimit += COLUMN_DELTA;
/*N*/ 
/*N*/ 		ColEntry* pNewItems = new ColEntry[nLimit];
/*N*/ 		if (pItems)
/*N*/ 		{
/*N*/ 			memmove( pNewItems, pItems, nCount * sizeof(ColEntry) );
/*N*/ 			delete[] pItems;
/*N*/ 		}
/*N*/ 		pItems = pNewItems;
/*N*/ 	}
/*N*/ 	pItems[nCount].pCell = pCell;
/*N*/ 	pItems[nCount].nRow  = nRow;
/*N*/ 	++nCount;
/*N*/ }


/*N*/ void ScColumn::DeleteAtIndex( USHORT nIndex )
/*N*/ {
/*N*/ 	ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 	ScNoteCell* pNoteCell = new ScNoteCell;
/*N*/ 	pItems[nIndex].pCell = pNoteCell;		// Dummy fuer Interpret
/*N*/ 	pDocument->Broadcast( ScHint( SC_HINT_DYING,
/*N*/ 		ScAddress( nCol, pItems[nIndex].nRow, nTab ), pCell ) );
/*N*/ 	delete pNoteCell;
/*N*/ 	--nCount;
/*N*/ 	memmove( &pItems[nIndex], &pItems[nIndex + 1], (nCount - nIndex) * sizeof(ColEntry) );
/*N*/ 	pItems[nCount].nRow = 0;
/*N*/ 	pItems[nCount].pCell = NULL;
/*N*/ 	pCell->EndListeningTo( pDocument );
/*N*/ 	pCell->Delete();
/*N*/ }


/*N*/ void ScColumn::FreeAll()
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 	{
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 			pItems[i].pCell->Delete();
/*N*/ 		delete[] pItems;
/*N*/ 		pItems = NULL;
/*N*/ 	}
/*N*/ 	nCount = 0;
/*N*/ 	nLimit = 0;
/*N*/ }


/*N*/ void ScColumn::DeleteRow( USHORT nStartRow, USHORT nSize )
/*N*/ {
/*N*/ 	pAttrArray->DeleteRow( nStartRow, nSize );
/*N*/ 
/*N*/ 	if ( !pItems || !nCount )
/*N*/ 		return ;
/*N*/ 
/*N*/ 	USHORT nFirstIndex;
/*N*/ 	Search( nStartRow, nFirstIndex );
/*N*/ 	if ( nFirstIndex >= nCount )
/*N*/ 		return ;
/*N*/ 
/*N*/ 	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/ 	pDocument->SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 
/*N*/ 	BOOL bFound=FALSE;
/*N*/ 	USHORT nEndRow = nStartRow + nSize - 1;
/*N*/ 	USHORT nStartIndex;
/*N*/ 	USHORT nEndIndex;
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	for ( i = nFirstIndex; i < nCount && pItems[i].nRow <= nEndRow; i++ )
/*N*/ 	{
/*N*/ 		if (!bFound)
/*N*/ 		{
/*N*/ 			nStartIndex = i;
/*N*/ 			bFound = TRUE;
/*N*/ 		}
/*N*/ 		nEndIndex = i;
/*N*/ 
/*N*/ 		ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 		ScBroadcasterList* pBC = pCell->GetBroadcaster();
/*N*/ 		if (pBC)
/*N*/ 		{
/*N*/ // gibt jetzt invalid reference, kein Aufruecken der direkten Referenzen
/*N*/ //			MoveListeners( *pBC, nRow+nSize );
/*N*/ 			pCell->SetBroadcaster(NULL);
/*N*/ 			//	in DeleteRange werden leere Broadcaster geloescht
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (bFound)
/*N*/ 	{
/*N*/ 		DeleteRange( nStartIndex, nEndIndex, IDF_CONTENTS );
/*N*/ 		Search( nStartRow, i );
/*N*/ 		if ( i >= nCount )
/*N*/ 		{
/*N*/ 			pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/ 			return ;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		i = nFirstIndex;
/*N*/ 
/*N*/ 	ScAddress aAdr( nCol, 0, nTab );
/*N*/     ScHint aHint( SC_HINT_DATACHANGED, aAdr, NULL );    // only areas (ScBaseCell* == NULL)
/*N*/     ScAddress& rAddress = aHint.GetAddress();
/*N*/     // for sparse occupation use single broadcasts, not ranges
/*N*/     BOOL bSingleBroadcasts = (((pItems[nCount-1].nRow - pItems[i].nRow) /
/*N*/                 (nCount - i)) > 1);
/*N*/     if ( bSingleBroadcasts )
/*N*/     {
/*N*/         USHORT nLastBroadcast = MAXROW+1;
/*N*/         for ( ; i < nCount; i++ )
/*N*/         {
/*N*/             USHORT nOldRow = pItems[i].nRow;
/*N*/             // #43940# Aenderung Quelle broadcasten
/*N*/             rAddress.SetRow( nOldRow );
/*N*/             pDocument->AreaBroadcast( aHint );
/*N*/             USHORT nNewRow = (pItems[i].nRow -= nSize);
/*N*/             // #43940# Aenderung Ziel broadcasten
/*N*/             if ( nLastBroadcast != nNewRow )
/*N*/             {   // direkt aufeinanderfolgende nicht doppelt broadcasten
/*N*/                 rAddress.SetRow( nNewRow );
/*N*/                 pDocument->AreaBroadcast( aHint );
/*N*/             }
/*N*/             nLastBroadcast = nOldRow;
/*N*/             ScBaseCell* pCell = pItems[i].pCell;
/*N*/             if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/                 ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         rAddress.SetRow( pItems[i].nRow );
/*N*/         ScRange aRange( rAddress );
/*N*/         aRange.aEnd.SetRow( pItems[nCount-1].nRow );
/*N*/         for ( ; i < nCount; i++ )
/*N*/         {
/*N*/             USHORT nNewRow = (pItems[i].nRow -= nSize);
/*N*/             ScBaseCell* pCell = pItems[i].pCell;
/*N*/             if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/                 ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
/*N*/         }
/*N*/         pDocument->AreaBroadcastInRange( aRange, aHint );
/*N*/     }
/*N*/ 
/*N*/ 	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScColumn::DeleteRange( USHORT nStartIndex, USHORT nEndIndex, USHORT nDelFlag )
/*N*/ {
/*N*/ 	USHORT nDelCount = 0;
/*N*/ 	ScBaseCell** ppDelCells = new ScBaseCell*[nEndIndex-nStartIndex+1];
/*N*/ 
/*N*/ 	BOOL bSimple = ((nDelFlag & IDF_CONTENTS) == IDF_CONTENTS);
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 		//	Notiz-Zeichenobjekte
/*N*/ 	if (nDelFlag & IDF_NOTE)
/*N*/ 	{
/*N*/ 		for ( i = nStartIndex; i <= nEndIndex; i++ )
/*N*/ 		{
/*N*/ 			const ScPostIt*	pNote = pItems[i].pCell->GetNotePtr();
/*N*/ 			if ( pNote && pNote->IsShown() )
/*N*/ 			{
/*?*/ 				ScDetectiveFunc( pDocument, nTab ).HideComment( nCol, pItems[i].nRow );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 		//	Broadcaster stehenlassen
/*N*/ 	if (bSimple)
/*N*/ 	{
/*N*/ 		for (i = nStartIndex; i <= nEndIndex && bSimple; i++)
/*N*/ 			if (pItems[i].pCell->GetBroadcaster())
/*N*/ 				bSimple = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/     ScHint aHint( SC_HINT_DYING, ScAddress( nCol, 0, nTab ), NULL );
/*N*/ 
/*N*/ 	if (bSimple)			// Bereich komplett loeschen
/*N*/ 	{
/*N*/ 		ScBaseCell* pOldCell;
/*N*/ 		ScNoteCell* pNoteCell = new ScNoteCell;		// Dummy
/*N*/ 		for (i = nStartIndex; i <= nEndIndex; i++)
/*N*/ 		{
/*N*/ 			pOldCell = pItems[i].pCell;
/*N*/ 			if (pOldCell->GetCellType() == CELLTYPE_FORMULA)		// Formeln spaeter loeschen
/*N*/ 				ppDelCells[nDelCount++] = pOldCell;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// Interpret in Broadcast darf kein Value finden
/*N*/ 				pItems[i].pCell = pNoteCell;
/*N*/                 aHint.GetAddress().SetRow( pItems[i].nRow );
/*N*/                 aHint.SetCell( pOldCell );
/*N*/ 				pDocument->Broadcast( aHint );
/*N*/ 				pOldCell->Delete();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		delete pNoteCell;
/*N*/ 		memmove( &pItems[nStartIndex], &pItems[nEndIndex + 1], (nCount - nEndIndex - 1) * sizeof(ColEntry) );
/*N*/ 		nCount -= nEndIndex-nStartIndex+1;
/*N*/ 	}
/*N*/ 	else					// Zellen einzeln durchgehen
/*N*/ 	{
/*N*/ 		USHORT j = nStartIndex;
/*N*/ 		for (USHORT i = nStartIndex; i <= nEndIndex; i++)
/*N*/ 		{
/*N*/ 			BOOL bDelete = FALSE;
/*N*/ 			ScBaseCell* pOldCell = pItems[j].pCell;
/*N*/ 			CellType eCellType = pOldCell->GetCellType();
/*N*/ 			switch ( eCellType )
/*N*/ 			{
/*N*/ 				case CELLTYPE_VALUE:
/*N*/ 					if ( ( nDelFlag & (IDF_DATETIME|IDF_VALUE) ) == (IDF_DATETIME|IDF_VALUE) )
/*N*/ 						bDelete = TRUE;
/*N*/ 					else
/*N*/ 					{
/*?*/ 						ULONG nIndex = (ULONG)((SfxUInt32Item*)GetAttr( pItems[j].nRow, ATTR_VALUE_FORMAT ))->GetValue();
/*?*/ 						short nTyp = pDocument->GetFormatTable()->GetType(nIndex);
/*?*/ 						if ((nTyp == NUMBERFORMAT_DATE) || (nTyp == NUMBERFORMAT_TIME) || (nTyp == NUMBERFORMAT_DATETIME))
/*?*/ 							bDelete = ((nDelFlag & IDF_DATETIME) != 0);
/*?*/ 						else
/*?*/ 							bDelete = ((nDelFlag & IDF_VALUE) != 0);
/*N*/ 					}
/*N*/ 					break;
/*?*/ 				case CELLTYPE_STRING:
/*N*/ 				case CELLTYPE_EDIT:		bDelete = ((nDelFlag & IDF_STRING) != 0); break;
/*?*/ 				case CELLTYPE_FORMULA:	bDelete = ((nDelFlag & IDF_FORMULA) != 0); break;
/*?*/ 				case CELLTYPE_NOTE:
/*?*/ 					bDelete = ((nDelFlag & IDF_NOTE) != 0) &&
/*?*/ 								(pOldCell->GetBroadcaster() == NULL);
/*?*/ 					break;
/*N*/ 			}
/*N*/ 
/*N*/ 			if (bDelete)
/*N*/ 			{
/*N*/ 				ScNoteCell* pNoteCell = NULL;
/*N*/ 				if (eCellType != CELLTYPE_NOTE)
/*N*/ 				{
/*N*/ 					if ((nDelFlag & IDF_NOTE) == 0)
/*N*/ 					{
/*?*/ 						const ScPostIt* pNote = pOldCell->GetNotePtr();
/*?*/ 						if (pNote)
/*?*/ 							pNoteCell = new ScNoteCell(*pNote);
/*N*/ 					}
/*N*/ 					ScBroadcasterList* pBC = pOldCell->GetBroadcaster();
/*N*/ 					if (pBC)
/*N*/ 					{
/*N*/ 						if (!pNoteCell)
/*N*/ 							pNoteCell = new ScNoteCell;
/*N*/ 						pNoteCell->SetBroadcaster(pBC);
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				USHORT nOldRow = pItems[j].nRow;
/*N*/ 				if (pNoteCell)
/*N*/ 				{
/*N*/ 					pItems[j].pCell = pNoteCell;
/*N*/ 					++j;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					--nCount;
/*N*/ 					memmove( &pItems[j], &pItems[j + 1], (nCount - j) * sizeof(ColEntry) );
/*N*/ 					pItems[nCount].nRow = 0;
/*N*/ 					pItems[nCount].pCell = NULL;
/*N*/ 				}
/*N*/ 				// ACHTUNG! pItems bereits verschoben!
/*N*/ 				// Interpret in Broadcast muss neue/keine Zelle finden
/*N*/ 				if (eCellType == CELLTYPE_FORMULA)			// Formeln spaeter loeschen
/*N*/ 				{
/*N*/ 					ppDelCells[nDelCount++] = pOldCell;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/                     aHint.GetAddress().SetRow( nOldRow );
/*N*/                     aHint.SetCell( pOldCell );
/*N*/ 					pDocument->Broadcast( aHint );
/*N*/ 					if (eCellType != CELLTYPE_NOTE)
/*N*/ 						pOldCell->ForgetBroadcaster();
/*N*/ 					pOldCell->Delete();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				if (nDelFlag & IDF_NOTE)
/*?*/ 					if (pItems[j].pCell->GetNotePtr())
/*?*/ 						pItems[j].pCell->DeleteNote();
/*?*/ 				++j;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// erst Listener abhaengen kann Neuberechnungen sparen
/*N*/ 	// eventuell werden dabei vorher entstandene NoteCell mitsamt
/*N*/ 	// ihren Broadcaster deleted!
/*N*/ 	for (i=0; i<nDelCount; i++)
/*N*/ 	{
/*?*/ 		((ScFormulaCell*) ppDelCells[i])->EndListeningTo( pDocument );
/*N*/ 	}
/*N*/ 	// gibts die NoteCell und damit den Broadcaster noch?
/*N*/ 	// If not, discard them all before broadcasting takes place!
/*N*/ 	for (i=0; i<nDelCount; i++)
/*N*/ 	{
/*?*/ 		ScFormulaCell* pOldCell = (ScFormulaCell*) ppDelCells[i];
/*?*/ 		USHORT nIndex;
/*?*/ 		if ( !Search( pOldCell->aPos.Row(), nIndex ) )
/*?*/ 			pOldCell->ForgetBroadcaster();
/*N*/ 	}
/*N*/ 	for (i=0; i<nDelCount; i++)
/*N*/ 	{
/*?*/ 		ScFormulaCell* pOldCell = (ScFormulaCell*) ppDelCells[i];
/*?*/         aHint.SetAddress( pOldCell->aPos );
/*?*/         aHint.SetCell( pOldCell );
/*?*/ 		pDocument->Broadcast( aHint );
/*?*/ 		pOldCell->ForgetBroadcaster();
/*?*/ 		pOldCell->Delete();
/*N*/ 	}
/*N*/ 
/*N*/ 	delete[] ppDelCells;
/*N*/ }


/*N*/ void ScColumn::DeleteArea(USHORT nStartRow, USHORT nEndRow, USHORT nDelFlag)
/*N*/ {
/*N*/ 	//	FreeAll darf hier nicht gerufen werden wegen Broadcastern
/*N*/ 
/*N*/ 	//	Attribute erst am Ende, damit vorher noch zwischen Zahlen und Datum
/*N*/ 	//	unterschieden werden kann (#47901#)
/*N*/ 
/*N*/ 	USHORT nContFlag = nDelFlag & IDF_CONTENTS;
/*N*/ 	if (pItems && nCount && nContFlag)
/*N*/ 	{
/*N*/ 		if (nStartRow==0 && nEndRow==MAXROW)
/*N*/ 			DeleteRange( 0, nCount-1, nContFlag );
/*N*/ 		else
/*N*/ 		{
/*?*/ 			BOOL bFound=FALSE;
/*?*/ 			USHORT nStartIndex;
/*?*/ 			USHORT nEndIndex;
/*?*/ 			for (USHORT i = 0; i < nCount; i++)
/*?*/ 				if ((pItems[i].nRow >= nStartRow) && (pItems[i].nRow <= nEndRow))
/*?*/ 				{
/*?*/ 					if (!bFound)
/*?*/ 					{
/*?*/ 						nStartIndex = i;
/*?*/ 						bFound = TRUE;
/*?*/ 					}
/*?*/ 					nEndIndex = i;
/*?*/ 				}
/*?*/ 			if (bFound)
/*?*/ 				DeleteRange( nStartIndex, nEndIndex, nContFlag );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( nDelFlag & IDF_EDITATTR )
/*N*/ 	{
/*?*/ 		DBG_ASSERT( nContFlag == 0, "DeleteArea: falsche Flags" );
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 		RemoveEditAttribs( nStartRow, nEndRow );
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Attribute erst hier
/*N*/ 	if ((nDelFlag & IDF_ATTRIB) == IDF_ATTRIB) pAttrArray->DeleteArea( nStartRow, nEndRow );
/*N*/ 	else if ((nDelFlag & IDF_ATTRIB) != 0) pAttrArray->DeleteHardAttr( nStartRow, nEndRow );
/*N*/ }




//	rColumn = Quelle
//	nRow1, nRow2 = Zielposition



    //	Formelzellen werden jetzt schon hier kopiert,
    //	Notizen muessen aber evtl. noch geloescht werden

/*N*/ ScBaseCell* ScColumn::CloneCell(USHORT nIndex, USHORT nFlags,
/*N*/ 									ScDocument* pDestDoc, const ScAddress& rDestPos)
/*N*/ {
/*N*/ 	ScBaseCell* pNew = 0;
/*N*/ 	ScBaseCell* pSource = pItems[nIndex].pCell;
/*N*/ 	switch (pSource->GetCellType())
/*N*/ 	{
/*?*/ 		case CELLTYPE_NOTE:
/*?*/ 			if (nFlags & IDF_NOTE)
/*?*/ 				pNew = new ScNoteCell(*(ScNoteCell*)pSource);
/*?*/ 			break;
/*?*/ 		case CELLTYPE_EDIT:
/*?*/ 			if (nFlags & IDF_STRING)
/*?*/ 				pNew = new ScEditCell( *(ScEditCell*)pSource, pDestDoc );
/*?*/ 			break;
/*N*/ 		case CELLTYPE_STRING:
/*N*/ 			if (nFlags & IDF_STRING)
/*N*/ 				pNew = new ScStringCell(*(ScStringCell*)pSource);
/*N*/ 			break;
/*N*/ 		case CELLTYPE_VALUE:
/*N*/ 			{
/*N*/ 				BOOL bDoIns = FALSE;
/*N*/ 				USHORT nMask = nFlags & ( IDF_DATETIME | IDF_VALUE );
/*N*/ 				if ( nMask == (IDF_DATETIME | IDF_VALUE) )
/*N*/ 					bDoIns = TRUE;
/*N*/ 				else if ( nMask )
/*N*/ 				{
/*?*/ 					ULONG nNumIndex = (ULONG)((SfxUInt32Item*) GetAttr(
/*?*/ 							pItems[nIndex].nRow, ATTR_VALUE_FORMAT ))->GetValue();
/*?*/ 					short nTyp = pDocument->GetFormatTable()->GetType(nNumIndex);
/*?*/ 					if (nTyp == NUMBERFORMAT_DATE || nTyp == NUMBERFORMAT_TIME || nTyp == NUMBERFORMAT_DATETIME)
/*?*/ 						bDoIns = (nFlags & IDF_DATETIME)!=0;
/*?*/ 					else
/*?*/ 						bDoIns = (nFlags & IDF_VALUE)!=0;
/*N*/ 				}
/*N*/ 				if (bDoIns)
/*N*/ 					pNew = new ScValueCell(*(ScValueCell*)pSource);
/*N*/ 			}
/*N*/ 			break;
/*?*/ 		case CELLTYPE_FORMULA:
/*?*/ 			{
/*?*/ 				ScFormulaCell* pForm = (ScFormulaCell*)pSource;
/*?*/ 				if (nFlags & IDF_FORMULA)
/*?*/ 				{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 					pNew = pForm->Clone( pDestDoc, rDestPos, TRUE );
/*?*/ 				}
/*?*/ 				else if ( (nFlags & (IDF_VALUE | IDF_DATETIME | IDF_STRING)) &&
/*?*/ 							!pDestDoc->IsUndo() )
/*?*/ 				{
/*?*/ 					//	#48491# ins Undo-Dokument immer nur die Original-Zelle kopieren,
/*?*/ 					//	aus Formeln keine Value/String-Zellen erzeugen
/*?*/ 
/*?*/ 					USHORT nErr = pForm->GetErrCode();
/*?*/ 					if ( nErr )
/*?*/ 					{
/*?*/ 						//	Fehler werden immer mit "Zahlen" kopiert
/*?*/ 						//	(Das ist hiermit willkuerlich so festgelegt)
/*?*/ 
/*?*/ 						if ( nFlags & IDF_VALUE )
/*?*/ 						{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 							ScFormulaCell* pErrCell = new ScFormulaCell( pDestDoc, rDestPos );
/*?*/ 						}
/*?*/ 					}
/*?*/ 					else if ( pForm->IsValue() )
/*?*/ 					{
/*?*/ 						BOOL bDoIns = FALSE;
/*?*/ 						USHORT nMask = nFlags & ( IDF_DATETIME | IDF_VALUE );
/*?*/ 						if ( nMask == (IDF_DATETIME | IDF_VALUE) )
/*?*/ 							bDoIns = TRUE;
/*?*/ 						else if ( nMask )
/*?*/ 						{
/*?*/ 							ULONG nNumIndex = (ULONG)((SfxUInt32Item*) GetAttr(
/*?*/ 									pItems[nIndex].nRow, ATTR_VALUE_FORMAT ))->GetValue();
/*?*/ 							short nTyp = pDocument->GetFormatTable()->GetType(nNumIndex);
/*?*/ 							if (nTyp == NUMBERFORMAT_DATE || nTyp == NUMBERFORMAT_TIME || nTyp == NUMBERFORMAT_DATETIME)
/*?*/ 								bDoIns = (nFlags & IDF_DATETIME)!=0;
/*?*/ 							else
/*?*/ 								bDoIns = (nFlags & IDF_VALUE)!=0;
/*?*/ 						}
/*?*/ 
/*?*/ 						if (bDoIns)
/*?*/ 						{
/*?*/ 							double nVal = pForm->GetValue();
/*?*/ 							pNew = new ScValueCell(nVal);
/*?*/ 						}
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						if (nFlags & IDF_STRING)
/*?*/ 						{
/*?*/ 							String aString;
/*?*/ 							pForm->GetString(aString);
/*?*/ 							if ( aString.Len() )
/*?*/ 								pNew = new ScStringCell(aString);
/*?*/ 								// #33224# LeerStrings nicht kopieren
/*?*/ 						}
/*?*/ 					}
/*?*/ 					if ( pNew && pSource->GetNotePtr() && ( nFlags & IDF_NOTE ) )
/*?*/ 						pNew->SetNote(*pSource->GetNotePtr());
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !pNew && pSource->GetNotePtr() && ( nFlags & IDF_NOTE ) )
/*?*/ 		pNew = new ScNoteCell(*pSource->GetNotePtr());
/*N*/ 
/*N*/ 	return pNew;
/*N*/ }




//	Ergebnis in rVal1







/*N*/ ScAttrIterator* ScColumn::CreateAttrIterator( USHORT nStartRow, USHORT nEndRow ) const
/*N*/ {
/*N*/ 	return new ScAttrIterator( pAttrArray, nStartRow, nEndRow );
/*N*/ }


/*N*/ void ScColumn::StartAllListeners()
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 			{
/*N*/ 				USHORT nRow = pItems[i].nRow;
/*N*/ 				((ScFormulaCell*)pCell)->StartListeningTo( pDocument );
/*N*/ 				if ( nRow != pItems[i].nRow )
/*N*/ 					Search( nRow, i );		// Listener eingefuegt?
/*N*/ 			}
/*N*/ 		}
/*N*/ }


/*N*/ void ScColumn::StartNameListeners( BOOL bOnlyRelNames )
/*N*/ {
/*N*/ 	if (pItems)
/*N*/     {
/*N*/         USHORT nNameListening = (bOnlyRelNames ? SC_LISTENING_NAMES_REL :
/*N*/             SC_LISTENING_NAMES_REL | SC_LISTENING_NAMES_ABS);
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 			{
/*N*/ 				USHORT nRow = pItems[i].nRow;
/*N*/                 ((ScFormulaCell*)pCell)->StartListeningTo( pDocument, nNameListening );
/*N*/ 				if ( nRow != pItems[i].nRow )
/*N*/ 					Search( nRow, i );		// Listener eingefuegt?
/*N*/ 			}
/*N*/ 		}
/*N*/     }
/*N*/ }






//	TRUE = Zahlformat gesetzt
/*N*/ BOOL ScColumn::SetString( USHORT nRow, USHORT nTab, const String& rString )
/*N*/ {
/*N*/ 	BOOL bNumFmtSet = FALSE;
/*N*/ 	if (VALIDROW(nRow))
/*N*/ 	{
/*N*/ 		ScBaseCell* pNewCell = NULL;
/*N*/ 		BOOL bIsLoading = FALSE;
/*N*/ 		if (rString.Len() > 0)
/*N*/ 		{
/*N*/ 			double nVal;
/*N*/ 			sal_uInt32 nIndex, nOldIndex;
/*N*/ 			sal_Unicode cFirstChar;
/*N*/ 			SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
/*N*/ 			SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
/*N*/ 			if ( pDocSh )
/*N*/ 				bIsLoading = pDocSh->IsLoading();
/*N*/ 			// IsLoading bei ConvertFrom Import
/*N*/ 			if ( !bIsLoading )
/*N*/ 			{
/*N*/ 				nIndex = nOldIndex = GetNumberFormat( nRow );
/*N*/ 				if ( rString.Len() > 1
/*N*/ 						&& pFormatter->GetType(nIndex) != NUMBERFORMAT_TEXT )
/*N*/ 					cFirstChar = rString.GetChar(0);
/*N*/ 				else
/*N*/ 					cFirstChar = 0;								// Text
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{	// waehrend ConvertFrom Import gibt es keine gesetzten Formate
/*N*/ 				cFirstChar = rString.GetChar(0);
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( cFirstChar == '=' )
/*N*/ 			{
/*?*/ 				if ( rString.Len() == 1 )						// = Text
/*?*/ 					pNewCell = new ScStringCell( rString );
/*?*/ 				else											// =Formel
/*?*/ 					pNewCell = new ScFormulaCell( pDocument,
/*?*/ 						ScAddress( nCol, nRow, nTab ), rString, 0 );
/*N*/ 			}
/*N*/ 			else if ( cFirstChar == '\'')						// 'Text
/*?*/ 				pNewCell = new ScStringCell( rString.Copy(1) );
/*N*/ 			else
/*N*/ 			{
/*N*/ 				BOOL bIsText = FALSE;
/*N*/ 				if ( bIsLoading )
/*N*/ 				{
/*N*/ 					if ( pItems && nCount )
/*N*/ 					{
/*?*/ 						String aStr;
/*?*/ 						USHORT i = nCount;
/*?*/ 						USHORT nStop = (i >= 3 ? i - 3 : 0);
/*?*/ 						// die letzten Zellen vergleichen, ob gleicher String
/*?*/ 						// und IsNumberFormat eingespart werden kann
/*?*/ 						do
/*?*/ 						{
/*?*/ 							i--;
/*?*/ 							ScBaseCell* pCell = pItems[i].pCell;
/*?*/ 							switch ( pCell->GetCellType() )
/*?*/ 							{
/*?*/ 								case CELLTYPE_STRING :
/*?*/ 									((ScStringCell*)pCell)->GetString( aStr );
/*?*/ 									if ( rString == aStr )
/*?*/ 										bIsText = TRUE;
/*?*/ 								break;
/*?*/ 								case CELLTYPE_NOTE :	// durch =Formel referenziert
/*?*/ 								break;
/*?*/ 								default:
/*?*/ 									if ( i == nCount - 1 )
/*?*/ 										i = 0;
/*?*/ 										// wahrscheinlich ganze Spalte kein String
/*?*/ 							}
/*?*/ 						} while ( i && i > nStop && !bIsText );
/*N*/ 					}
/*N*/ 					// nIndex fuer IsNumberFormat vorbelegen
/*N*/ 					if ( !bIsText )
/*N*/ 						nIndex = nOldIndex = pFormatter->GetStandardIndex();
/*N*/ 				}
/*N*/ 				if ( !bIsText &&
/*N*/ 						pFormatter->IsNumberFormat(rString, nIndex, nVal) )
/*N*/ 				{												// Zahl
/*?*/ 					pNewCell = new ScValueCell( nVal );
/*?*/ 					if ( nIndex != nOldIndex)
/*?*/ 					{
/*?*/ 						ApplyAttr( nRow, SfxUInt32Item( ATTR_VALUE_FORMAT,
/*?*/ 							(UINT32) nIndex) );
/*?*/ 						bNumFmtSet = TRUE;
/*?*/ 					}
/*N*/ 				}
/*N*/ 				else											// Text
/*N*/ 					pNewCell = new ScStringCell( rString );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( bIsLoading && (!nCount || nRow > pItems[nCount-1].nRow) )
/*N*/ 		{	// Search einsparen und ohne Umweg ueber Insert, Listener aufbauen
/*N*/ 			// und Broadcast kommt eh erst nach dem Laden
/*N*/ 			if ( pNewCell )
/*N*/ 				Append( nRow, pNewCell );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			USHORT i;
/*N*/ 			if (Search(nRow, i))
/*N*/ 			{
/*?*/ 				ScBaseCell* pOldCell = pItems[i].pCell;
/*?*/ 				const ScPostIt* pNote = pOldCell->GetNotePtr();
/*?*/ 				ScBroadcasterList* pBC = pOldCell->GetBroadcaster();
/*?*/ 				if (pNewCell || pNote || pBC)
/*?*/ 				{
/*?*/ 					if (!pNewCell)
/*?*/ 						pNewCell = new ScNoteCell();
/*?*/ 					if (pNote)
/*?*/ 						pNewCell->SetNote(*pNote);
/*?*/ 					if (pBC)
/*?*/ 					{
/*?*/ 						pNewCell->SetBroadcaster(pBC);
/*?*/ 						pOldCell->ForgetBroadcaster();
/*?*/ 						pLastFormulaTreeTop = 0;	// Err527 Workaround
/*?*/ 					}
/*?*/ 
/*?*/ 					if ( pOldCell->GetCellType() == CELLTYPE_FORMULA )
/*?*/ 					{
/*?*/ 						pOldCell->EndListeningTo( pDocument );
/*?*/ 						// falls in EndListening NoteCell in gleicher Col zerstoert
/*?*/ 						if ( i >= nCount || pItems[i].nRow != nRow )
/*?*/ 							Search(nRow, i);
/*?*/ 					}
/*?*/ 					pOldCell->Delete();
/*?*/ 					pItems[i].pCell = pNewCell;			// ersetzen
/*?*/ 					if ( pNewCell->GetCellType() == CELLTYPE_FORMULA )
/*?*/ 					{
/*?*/ 						pNewCell->StartListeningTo( pDocument );
/*?*/ 						((ScFormulaCell*)pNewCell)->SetDirty();
/*?*/ 					}
/*?*/ 					else
/*?*/ 						pDocument->Broadcast( ScHint( SC_HINT_DATACHANGED,
/*?*/ 							ScAddress( nCol, nRow, nTab ), pNewCell ) );
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					DeleteAtIndex(i);					// loeschen und Broadcast
/*?*/ 				}
/*N*/ 			}
/*N*/ 			else if (pNewCell)
/*N*/ 			{
/*N*/ 				Insert(nRow, pNewCell);					// neu eintragen und Broadcast
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//	hier keine Formate mehr fuer Formeln setzen!
/*N*/ 		//	(werden bei der Ausgabe abgefragt)
/*N*/ 
/*N*/ 	}
/*N*/ 	return bNumFmtSet;
/*N*/ }



//
//	GetDataEntries - Strings aus zusammenhaengendem Bereich um nRow
//

//	DATENT_MAX		- max. Anzahl Eintrage in Liste fuer Auto-Eingabe
//	DATENT_SEARCH	- max. Anzahl Zellen, die durchsucht werden - neu: nur Strings zaehlen
#define DATENT_MAX		200
#define DATENT_SEARCH	2000



#undef DATENT_MAX
#undef DATENT_SEARCH




/*N*/ void ScColumn::SetError( USHORT nRow, const USHORT nError)
/*N*/ {
/*N*/ 	if (VALIDROW(nRow))
/*N*/ 	{
/*N*/ 		ScFormulaCell* pCell = new ScFormulaCell
/*N*/ 			( pDocument, ScAddress( nCol, nRow, nTab ) );
/*N*/ 		pCell->SetErrCode( nError );
/*N*/ 		Insert( nRow, pCell );
/*N*/ 	}
/*N*/ }


/*N*/ void ScColumn::SetValue( USHORT nRow, const double& rVal)
/*N*/ {
/*N*/ 	if (VALIDROW(nRow))
/*N*/ 	{
/*N*/ 		ScBaseCell* pCell = new ScValueCell(rVal);
/*N*/ 		Insert( nRow, pCell );
/*N*/ 	}
/*N*/ }


/*N*/ void ScColumn::SetNote( USHORT nRow, const ScPostIt& rNote)
/*N*/ {
/*N*/ 	BOOL bEmpty = !rNote.GetText().Len();
/*N*/ 
/*N*/ 	USHORT nIndex;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 	{
/*N*/ 		ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 		if (bEmpty && pCell->GetCellType() == CELLTYPE_NOTE && !pCell->GetBroadcaster())
/*?*/ 			DeleteAtIndex(nIndex);
/*N*/ 		else
/*N*/ 			pCell->SetNote(rNote);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (!bEmpty)
/*N*/ 			Insert(nRow, new ScNoteCell(rNote));
/*N*/ 	}
/*N*/ }


/*N*/ void ScColumn::GetString( USHORT nRow, String& rString ) const
/*N*/ {
/*N*/ 	USHORT	nIndex;
/*N*/ 	Color* pColor;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 	{
/*N*/ 		ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 		if (pCell->GetCellType() != CELLTYPE_NOTE)
/*N*/ 		{
/*N*/ 			ULONG nFormat = GetNumberFormat( nRow );
/*N*/ 			ScCellFormat::GetString( pCell, nFormat, rString, &pColor, *(pDocument->GetFormatTable()) );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			rString.Erase();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rString.Erase();
/*N*/ }


/*N*/ void ScColumn::GetInputString( USHORT nRow, String& rString ) const
/*N*/ {
/*N*/ 	USHORT	nIndex;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 	{
/*?*/ 		ScBaseCell* pCell = pItems[nIndex].pCell;
/*?*/ 		if (pCell->GetCellType() != CELLTYPE_NOTE)
/*?*/ 		{
/*?*/ 			ULONG nFormat = GetNumberFormat( nRow );
/*?*/ 			ScCellFormat::GetInputString( pCell, nFormat, rString, *(pDocument->GetFormatTable()) );
/*?*/ 		}
/*?*/ 		else
/*?*/ 			rString.Erase();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rString.Erase();
/*N*/ }


/*N*/ double ScColumn::GetValue( USHORT nRow ) const
/*N*/ {
/*N*/ 	USHORT	nIndex;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 	{
/*N*/ 		ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 		switch (pCell->GetCellType())
/*N*/ 		{
/*N*/ 			case CELLTYPE_VALUE:
/*N*/ 				return ((ScValueCell*)pCell)->GetValue();
/*N*/ 				break;
/*N*/ 			case CELLTYPE_FORMULA:
/*N*/ 				{
/*N*/ 					if (((ScFormulaCell*)pCell)->IsValue())
/*N*/ 						return ((ScFormulaCell*)pCell)->GetValue();
/*N*/ 					else
/*N*/ 						return 0.0;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				return 0.0;
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0.0;
/*N*/ }

/*N*/ BOOL ScColumn::GetNote( USHORT nRow, ScPostIt& rNote) const
/*N*/ {
/*N*/ 	BOOL	bHasNote = FALSE;
/*N*/ 	USHORT	nIndex;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 		bHasNote = pItems[nIndex].pCell->GetNote(rNote);
/*N*/ 	else
/*?*/ 		rNote.Clear();
/*N*/ 
/*N*/ 	return bHasNote;
/*N*/ }


/*N*/ CellType ScColumn::GetCellType( USHORT nRow ) const
/*N*/ {
/*N*/ 	USHORT	nIndex;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 		return pItems[nIndex].pCell->GetCellType();
/*N*/ 	return CELLTYPE_NONE;
/*N*/ }


/*N*/ USHORT ScColumn::GetErrCode( USHORT nRow ) const
/*N*/ {
/*N*/  	USHORT	nIndex;
/*N*/  	if (Search(nRow, nIndex))
/*N*/  	{
/*N*/  		ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/  		if (pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/  			return ((ScFormulaCell*)pCell)->GetErrCode();
/*N*/  	}
/*N*/ 	return 0;
/*N*/ }


/*N*/ BOOL ScColumn::HasStringData( USHORT nRow ) const
/*N*/ {
/*N*/ 	USHORT	nIndex;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 		return (pItems[nIndex].pCell)->HasStringData();
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ BOOL ScColumn::HasValueData( USHORT nRow ) const
/*N*/ {
/*N*/ 	USHORT	nIndex;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 		return (pItems[nIndex].pCell)->HasValueData();
/*N*/ 	return FALSE;
/*N*/ }






}
