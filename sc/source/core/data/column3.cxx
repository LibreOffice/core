/*************************************************************************
 *
 *  $RCSfile: column3.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 18:05:33 $
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
// INCLUDE ---------------------------------------------------------------

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <sfx2/objsh.hxx>
#include <svtools/zforlist.hxx>

#include "scitems.hxx"
#include "column.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "collect.hxx"
#include "compiler.hxx"
#include "brdcst.hxx"
#include "docoptio.hxx"         // GetStdPrecision fuer GetMaxStringLen
#include "subtotal.hxx"
#include "markdata.hxx"
#include "detfunc.hxx"          // fuer Notizen bei DeleteRange

// Err527 Workaround
extern const ScFormulaCell* pLastFormulaTreeTop;    // in cellform.cxx

// STATIC DATA -----------------------------------------------------------

BOOL ScColumn::bDoubleAlloc = FALSE;    // fuer Import: Groesse beim Allozieren verdoppeln


void ScColumn::Insert( USHORT nRow, ScBaseCell* pNewCell )
{
    BOOL bIsAppended = FALSE;
    if (pItems && nCount)
    {
        if (pItems[nCount-1].nRow < nRow)
        {
            Append(nRow, pNewCell );
            bIsAppended = TRUE;
        }
    }
    if ( !bIsAppended )
    {
        USHORT  nIndex;
        if (Search(nRow, nIndex))
        {
            ScBaseCell* pOldCell = pItems[nIndex].pCell;
            ScBroadcasterList* pBC = pOldCell->GetBroadcaster();
            if (pBC && !pNewCell->GetBroadcaster())
            {
                pNewCell->SetBroadcaster( pBC );
                pOldCell->ForgetBroadcaster();
            }
            if (pOldCell->GetNotePtr() && !pNewCell->GetNotePtr())
                pNewCell->SetNote( *pOldCell->GetNotePtr() );
            if ( pOldCell->GetCellType() == CELLTYPE_FORMULA && !pDocument->IsClipOrUndo() )
            {
                pOldCell->EndListeningTo( pDocument );
                // falls in EndListening NoteCell in gleicher Col zerstoert
                if ( nIndex >= nCount || pItems[nIndex].nRow != nRow )
                    Search(nRow, nIndex);
            }
            pOldCell->Delete();
            pItems[nIndex].pCell = pNewCell;
        }
        else
        {
            if (nCount + 1 > nLimit)
            {
                if (bDoubleAlloc)
                {
                    if (nLimit < COLUMN_DELTA)
                        nLimit = COLUMN_DELTA;
                    else
                    {
                        nLimit *= 2;
                        if ( nLimit > MAXROW+1 )
                            nLimit = MAXROW+1;
                    }
                }
                else
                    nLimit += COLUMN_DELTA;

                ColEntry* pNewItems = new ColEntry[nLimit];
                if (pItems)
                {
                    memmove( pNewItems, pItems, nCount * sizeof(ColEntry) );
                    delete[] pItems;
                }
                pItems = pNewItems;
            }
            memmove( &pItems[nIndex + 1], &pItems[nIndex], (nCount - nIndex) * sizeof(ColEntry) );
            pItems[nIndex].pCell = pNewCell;
            pItems[nIndex].nRow  = nRow;
            ++nCount;
        }
    }
    // Bei aus Clipboard sind hier noch falsche (alte) Referenzen!
    // Werden in CopyBlockFromClip per UpdateReference umgesetzt,
    // danach StartListeningFromClip und BroadcastFromClip gerufen.
    // Wird ins Clipboard/UndoDoc gestellt, wird kein Broadcast gebraucht.
    // Nach Import wird CalcAfterLoad gerufen, dort Listening.
    if ( !(pDocument->IsClipOrUndo() || pDocument->IsInsertingFromOtherDoc()) )
    {
        pNewCell->StartListeningTo( pDocument );
        CellType eCellType = pNewCell->GetCellType();
        // Notizzelle entsteht beim Laden nur durch StartListeningCell,
        // ausloesende Formelzelle muss sowieso dirty sein.
        if ( !(pDocument->IsCalcingAfterLoad() && eCellType == CELLTYPE_NOTE) )
        {
            if ( eCellType == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pNewCell)->SetDirty();
            else
                pDocument->Broadcast( SC_HINT_DATACHANGED,
                    ScAddress( nCol, nRow, nTab ), pNewCell );
        }
    }
}


void ScColumn::Insert( USHORT nRow, ULONG nNumberFormat, ScBaseCell* pCell )
{
    Insert(nRow, pCell);
    short eOldType = pDocument->GetFormatTable()->
                        GetType( (ULONG)
                            ((SfxUInt32Item*)GetAttr( nRow, ATTR_VALUE_FORMAT ))->
                                GetValue() );
    short eNewType = pDocument->GetFormatTable()->GetType(nNumberFormat);
    if (!pDocument->GetFormatTable()->IsCompatible(eOldType, eNewType))
        ApplyAttr( nRow, SfxUInt32Item( ATTR_VALUE_FORMAT, (UINT32) nNumberFormat) );
}


void ScColumn::Append( USHORT nRow, ScBaseCell* pCell )
{
    if (nCount + 1 > nLimit)
    {
        if (bDoubleAlloc)
        {
            if (nLimit < COLUMN_DELTA)
                nLimit = COLUMN_DELTA;
            else
            {
                nLimit *= 2;
                if ( nLimit > MAXROW+1 )
                    nLimit = MAXROW+1;
            }
        }
        else
            nLimit += COLUMN_DELTA;

        ColEntry* pNewItems = new ColEntry[nLimit];
        if (pItems)
        {
            memmove( pNewItems, pItems, nCount * sizeof(ColEntry) );
            delete[] pItems;
        }
        pItems = pNewItems;
    }
    pItems[nCount].pCell = pCell;
    pItems[nCount].nRow  = nRow;
    ++nCount;
}


void ScColumn::Delete( USHORT nRow )
{
    USHORT  nIndex;

    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        ScNoteCell* pNoteCell = new ScNoteCell;
        pItems[nIndex].pCell = pNoteCell;       // Dummy fuer Interpret
        pDocument->Broadcast( SC_HINT_DYING,
            ScAddress( nCol, nRow, nTab ), pCell );
        ScBroadcasterList* pBC = pCell->GetBroadcaster();
        if ( pBC )
        {
            pNoteCell->SetBroadcaster( pBC );
            pCell->ForgetBroadcaster();
        }
        else
        {
            delete pNoteCell;
            --nCount;
            memmove( &pItems[nIndex], &pItems[nIndex + 1], (nCount - nIndex) * sizeof(ColEntry) );
            pItems[nCount].nRow = 0;
            pItems[nCount].pCell = NULL;
            //  Soll man hier den Speicher freigeben (delta)? Wird dann langsamer!
        }
        pCell->EndListeningTo( pDocument );
        pCell->Delete();
    }
}


void ScColumn::DeleteAtIndex( USHORT nIndex )
{
    ScBaseCell* pCell = pItems[nIndex].pCell;
    ScNoteCell* pNoteCell = new ScNoteCell;
    pItems[nIndex].pCell = pNoteCell;       // Dummy fuer Interpret
    pDocument->Broadcast( SC_HINT_DYING,
        ScAddress( nCol, pItems[nIndex].nRow, nTab ), pCell );
    delete pNoteCell;
    --nCount;
    memmove( &pItems[nIndex], &pItems[nIndex + 1], (nCount - nIndex) * sizeof(ColEntry) );
    pItems[nCount].nRow = 0;
    pItems[nCount].pCell = NULL;
    pCell->EndListeningTo( pDocument );
    pCell->Delete();
}


void ScColumn::FreeAll()
{
    if (pItems)
    {
        for (USHORT i = 0; i < nCount; i++)
            pItems[i].pCell->Delete();
        delete[] pItems;
        pItems = NULL;
    }
    nCount = 0;
    nLimit = 0;
}


void ScColumn::DeleteRow( USHORT nStartRow, USHORT nSize )
{
    pAttrArray->DeleteRow( nStartRow, nSize );

    if ( !pItems || !nCount )
        return ;

    USHORT nFirstIndex;
    Search( nStartRow, nFirstIndex );
    if ( nFirstIndex >= nCount )
        return ;

    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden

    BOOL bFound=FALSE;
    USHORT nEndRow = nStartRow + nSize - 1;
    USHORT nStartIndex;
    USHORT nEndIndex;
    USHORT i;

    for ( i = nFirstIndex; i < nCount && pItems[i].nRow <= nEndRow; i++ )
    {
        if (!bFound)
        {
            nStartIndex = i;
            bFound = TRUE;
        }
        nEndIndex = i;

        ScBaseCell* pCell = pItems[i].pCell;
        ScBroadcasterList* pBC = pCell->GetBroadcaster();
        if (pBC)
        {
// gibt jetzt invalid reference, kein Aufruecken der direkten Referenzen
//          MoveListeners( *pBC, nRow+nSize );
            pCell->SetBroadcaster(NULL);
            //  in DeleteRange werden leere Broadcaster geloescht
        }
    }
    if (bFound)
    {
        DeleteRange( nStartIndex, nEndIndex, IDF_CONTENTS );
        Search( nStartRow, i );
        if ( i >= nCount )
        {
            pDocument->SetAutoCalc( bOldAutoCalc );
            return ;
        }
    }
    else
        i = nFirstIndex;

    USHORT nLastBroadcast = MAXROW+1;
    ScAddress aAdr( nCol, 0, nTab );

    for ( ; i < nCount; i++ )
    {
        USHORT nOldRow = pItems[i].nRow;
        // #43940# Aenderung Quelle broadcasten
        aAdr.SetRow( nOldRow );
        pDocument->Broadcast( SC_HINT_DATACHANGED, aAdr, NULL );    // nur Areas
        USHORT nNewRow = (pItems[i].nRow -= nSize);
        // #43940# Aenderung Ziel broadcasten
        if ( nLastBroadcast != nNewRow )
        {   // direkt aufeinanderfolgende nicht doppelt broadcasten
            aAdr.SetRow( nNewRow );
            pDocument->Broadcast( SC_HINT_DATACHANGED, aAdr, NULL );    // nur Areas
        }
        nLastBroadcast = nOldRow;
        ScBaseCell* pCell = pItems[i].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
    }

    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::DeleteRange( USHORT nStartIndex, USHORT nEndIndex, USHORT nDelFlag )
{
    USHORT nDelCount = 0;
    ScBaseCell** ppDelCells = new ScBaseCell*[nEndIndex-nStartIndex+1];

    BOOL bSimple = ((nDelFlag & IDF_CONTENTS) == IDF_CONTENTS);
    USHORT i;

        //  Notiz-Zeichenobjekte
    if (nDelFlag & IDF_NOTE)
    {
        for ( i = nStartIndex; i <= nEndIndex; i++ )
        {
            const ScPostIt* pNote = pItems[i].pCell->GetNotePtr();
            if ( pNote && pNote->IsShown() )
            {
                ScDetectiveFunc( pDocument, nTab ).HideComment( nCol, pItems[i].nRow );
            }
        }
    }

        //  Broadcaster stehenlassen
    if (bSimple)
    {
        for (i = nStartIndex; i <= nEndIndex && bSimple; i++)
            if (pItems[i].pCell->GetBroadcaster())
                bSimple = FALSE;
    }

    if (bSimple)            // Bereich komplett loeschen
    {
        ScBaseCell* pOldCell;
        ScNoteCell* pNoteCell = new ScNoteCell;     // Dummy
        for (i = nStartIndex; i <= nEndIndex; i++)
        {
            pOldCell = pItems[i].pCell;
            if (pOldCell->GetCellType() == CELLTYPE_FORMULA)        // Formeln spaeter loeschen
                ppDelCells[nDelCount++] = pOldCell;
            else
            {
                // Interpret in Broadcast darf kein Value finden
                pItems[i].pCell = pNoteCell;
                pDocument->Broadcast( SC_HINT_DYING,
                    ScAddress( nCol, pItems[i].nRow, nTab ), pOldCell );
                pOldCell->Delete();
            }
        }
        delete pNoteCell;
        memmove( &pItems[nStartIndex], &pItems[nEndIndex + 1], (nCount - nEndIndex - 1) * sizeof(ColEntry) );
        nCount -= nEndIndex-nStartIndex+1;
    }
    else                    // Zellen einzeln durchgehen
    {
        USHORT j = nStartIndex;
        for (USHORT i = nStartIndex; i <= nEndIndex; i++)
        {
            BOOL bDelete = FALSE;
            ScBaseCell* pOldCell = pItems[j].pCell;
            CellType eCellType = pOldCell->GetCellType();
            switch ( eCellType )
            {
                case CELLTYPE_VALUE:
                    if ( ( nDelFlag & (IDF_DATETIME|IDF_VALUE) ) == (IDF_DATETIME|IDF_VALUE) )
                        bDelete = TRUE;
                    else
                    {
                        ULONG nIndex = (ULONG)((SfxUInt32Item*)GetAttr( pItems[j].nRow, ATTR_VALUE_FORMAT ))->GetValue();
                        short nTyp = pDocument->GetFormatTable()->GetType(nIndex);
                        if ((nTyp == NUMBERFORMAT_DATE) || (nTyp == NUMBERFORMAT_TIME) || (nTyp == NUMBERFORMAT_DATETIME))
                            bDelete = ((nDelFlag & IDF_DATETIME) != 0);
                        else
                            bDelete = ((nDelFlag & IDF_VALUE) != 0);
                    }
                    break;
                case CELLTYPE_STRING:
                case CELLTYPE_EDIT:     bDelete = ((nDelFlag & IDF_STRING) != 0); break;
                case CELLTYPE_FORMULA:  bDelete = ((nDelFlag & IDF_FORMULA) != 0); break;
                case CELLTYPE_NOTE:
                    bDelete = ((nDelFlag & IDF_NOTE) != 0) &&
                                (pOldCell->GetBroadcaster() == NULL);
                    break;
            }

            if (bDelete)
            {
                ScNoteCell* pNoteCell = NULL;
                if (eCellType != CELLTYPE_NOTE)
                {
                    if ((nDelFlag & IDF_NOTE) == 0)
                    {
                        const ScPostIt* pNote = pOldCell->GetNotePtr();
                        if (pNote)
                            pNoteCell = new ScNoteCell(*pNote);
                    }
                    ScBroadcasterList* pBC = pOldCell->GetBroadcaster();
                    if (pBC)
                    {
                        if (!pNoteCell)
                            pNoteCell = new ScNoteCell;
                        pNoteCell->SetBroadcaster(pBC);
                    }
                }

                USHORT nOldRow = pItems[j].nRow;
                if (pNoteCell)
                {
                    pItems[j].pCell = pNoteCell;
                    ++j;
                }
                else
                {
                    --nCount;
                    memmove( &pItems[j], &pItems[j + 1], (nCount - j) * sizeof(ColEntry) );
                    pItems[nCount].nRow = 0;
                    pItems[nCount].pCell = NULL;
                }
                // ACHTUNG! pItems bereits verschoben!
                // Interpret in Broadcast muss neue/keine Zelle finden
                if (eCellType == CELLTYPE_FORMULA)          // Formeln spaeter loeschen
                {
                    ppDelCells[nDelCount++] = pOldCell;
                }
                else
                {
                    pDocument->Broadcast( SC_HINT_DYING,
                        ScAddress( nCol, nOldRow, nTab ), pOldCell );
                    if (eCellType != CELLTYPE_NOTE)
                        pOldCell->ForgetBroadcaster();
                    pOldCell->Delete();
                }
            }
            else
            {
                if (nDelFlag & IDF_NOTE)
                    if (pItems[j].pCell->GetNotePtr())
                        pItems[j].pCell->DeleteNote();
                ++j;
            }
        }
    }

    // erst Listener abhaengen kann Neuberechnungen sparen
    // eventuell werden dabei vorher entstandene NoteCell mitsamt
    // ihren Broadcaster deleted!
    for (i=0; i<nDelCount; i++)
    {
        ((ScFormulaCell*) ppDelCells[i])->EndListeningTo( pDocument );
    }
    // gibts die NoteCell und damit den Broadcaster noch?
    // If not, discard them all before broadcasting takes place!
    for (i=0; i<nDelCount; i++)
    {
        ScFormulaCell* pOldCell = (ScFormulaCell*) ppDelCells[i];
        USHORT nIndex;
        if ( !Search( pOldCell->aPos.Row(), nIndex ) )
            pOldCell->ForgetBroadcaster();
    }
    for (i=0; i<nDelCount; i++)
    {
        ScFormulaCell* pOldCell = (ScFormulaCell*) ppDelCells[i];
        pDocument->Broadcast( SC_HINT_DYING, pOldCell->aPos, pOldCell );
        pOldCell->ForgetBroadcaster();
        pOldCell->Delete();
    }
    delete ppDelCells;
}


void ScColumn::DeleteArea(USHORT nStartRow, USHORT nEndRow, USHORT nDelFlag)
{
    //  FreeAll darf hier nicht gerufen werden wegen Broadcastern

    //  Attribute erst am Ende, damit vorher noch zwischen Zahlen und Datum
    //  unterschieden werden kann (#47901#)

    USHORT nContFlag = nDelFlag & IDF_CONTENTS;
    if (pItems && nCount && nContFlag)
    {
        if (nStartRow==0 && nEndRow==MAXROW)
            DeleteRange( 0, nCount-1, nContFlag );
        else
        {
            BOOL bFound=FALSE;
            USHORT nStartIndex;
            USHORT nEndIndex;
            for (USHORT i = 0; i < nCount; i++)
                if ((pItems[i].nRow >= nStartRow) && (pItems[i].nRow <= nEndRow))
                {
                    if (!bFound)
                    {
                        nStartIndex = i;
                        bFound = TRUE;
                    }
                    nEndIndex = i;
                }
            if (bFound)
                DeleteRange( nStartIndex, nEndIndex, nContFlag );
        }
    }

    if ( nDelFlag & IDF_EDITATTR )
    {
        DBG_ASSERT( nContFlag == 0, "DeleteArea: falsche Flags" );
        RemoveEditAttribs( nStartRow, nEndRow );
    }

    //  Attribute erst hier
    if ((nDelFlag & IDF_ATTRIB) == IDF_ATTRIB) pAttrArray->DeleteArea( nStartRow, nEndRow );
    else if ((nDelFlag & IDF_ATTRIB) != 0) pAttrArray->DeleteHardAttr( nStartRow, nEndRow );
}


ScFormulaCell* ScColumn::CreateRefCell( ScDocument* pDestDoc, const ScAddress& rDestPos,
                                            USHORT nIndex, USHORT nFlags ) const
{
    USHORT nContFlags = nFlags & IDF_CONTENTS;
    if (!nContFlags)
        return NULL;

    //  Testen, ob Zelle kopiert werden soll
    //  auch bei IDF_CONTENTS komplett, wegen Notes / Broadcastern

    BOOL bMatch = FALSE;
    ScBaseCell* pCell = pItems[nIndex].pCell;
    CellType eCellType = pCell->GetCellType();
    switch ( eCellType )
    {
        case CELLTYPE_VALUE:
            {
                USHORT nValFlags = nFlags & (IDF_DATETIME|IDF_VALUE);

                if ( nValFlags == (IDF_DATETIME|IDF_VALUE) )
                    bMatch = TRUE;
                else if ( nValFlags )
                {
                    ULONG nNumIndex = (ULONG)((SfxUInt32Item*)GetAttr(
                                    pItems[nIndex].nRow, ATTR_VALUE_FORMAT ))->GetValue();
                    short nTyp = pDocument->GetFormatTable()->GetType(nNumIndex);
                    if ((nTyp == NUMBERFORMAT_DATE) || (nTyp == NUMBERFORMAT_TIME) || (nTyp == NUMBERFORMAT_DATETIME))
                        bMatch = ((nFlags & IDF_DATETIME) != 0);
                    else
                        bMatch = ((nFlags & IDF_VALUE) != 0);
                }
            }
            break;
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:     bMatch = ((nFlags & IDF_STRING) != 0); break;
        case CELLTYPE_FORMULA:  bMatch = ((nFlags & IDF_FORMULA) != 0); break;
    }
    if (!bMatch)
        return NULL;


    //  Referenz einsetzen
    SingleRefData aRef;
    aRef.nCol = nCol;
    aRef.nRow = pItems[nIndex].nRow;
    aRef.nTab = nTab;
    aRef.InitFlags();                           // -> alles absolut
    aRef.SetFlag3D(TRUE);

    //! 3D(FALSE) und TabRel(TRUE), wenn die endgueltige Position auf der selben Tabelle ist?
    //! (bei TransposeClip ist die Zielposition noch nicht bekannt)

    aRef.CalcRelFromAbs( rDestPos );

    ScTokenArray aArr;
    aArr.AddSingleReference( aRef );

    return new ScFormulaCell( pDestDoc, rDestPos, &aArr );
}


//  rColumn = Quelle
//  nRow1, nRow2 = Zielposition

void ScColumn::CopyFromClip(USHORT nRow1, USHORT nRow2, short nDy,
                                USHORT nInsFlag, BOOL bAsLink, ScColumn& rColumn)
{
    if ((nInsFlag & IDF_ATTRIB) != 0) rColumn.pAttrArray->CopyAreaSafe( nRow1, nRow2, nDy, *pAttrArray );
    if ((nInsFlag & IDF_CONTENTS) == 0)
        return;

    if ( bAsLink && nInsFlag == IDF_ALL )
    {
        //  bei "alles" werden auch leere Zellen referenziert
        //! IDF_ALL muss immer mehr Flags enthalten, als bei "Inhalte Einfuegen"
        //! einzeln ausgewaehlt werden koennen!

        Resize( nCount + (nRow2-nRow1+1) );

        ScAddress aDestPos( nCol, 0, nTab );        // Row wird angepasst

        //  Referenz erzeugen (Quell-Position)
        SingleRefData aRef;
        aRef.nCol = rColumn.nCol;
        //  nRow wird angepasst
        aRef.nTab = rColumn.nTab;
        aRef.InitFlags();                           // -> alles absolut
        aRef.SetFlag3D(TRUE);

        for (USHORT nDestRow = nRow1; nDestRow <= nRow2; nDestRow++)
        {
            aRef.nRow = nDestRow - nDy;             // Quell-Zeile
            aDestPos.SetRow( nDestRow );

            aRef.CalcRelFromAbs( aDestPos );
            ScTokenArray aArr;
            aArr.AddSingleReference( aRef );
            Insert( nDestRow, new ScFormulaCell( pDocument, aDestPos, &aArr ) );
        }

        return;
    }

    USHORT nColCount = rColumn.nCount;
    if ((nInsFlag & IDF_CONTENTS) == IDF_CONTENTS && nRow2-nRow1 >= 64)
    {
        //! Resize immer aussen, wenn die Wiederholungen bekannt sind
        //! (dann hier gar nicht mehr)

        USHORT nNew = nCount + nColCount;
        if ( nLimit < nNew )
            Resize( nNew );
    }

    USHORT nRowDiff = nRow2 - nRow1;
    USHORT nDestRow;
    for (USHORT i = 0; (i < nColCount) && (i <= nRowDiff); i++)
    {
        if ( (nDestRow = rColumn.pItems[i].nRow + nDy) <= nRow2 )
        {
            ScBaseCell* pOld = rColumn.pItems[i].pCell;
            ScBaseCell* pNew;

            if ( bAsLink )
            {
                pNew = rColumn.CreateRefCell( pDocument,
                        ScAddress( nCol, nDestRow, nTab ), i, nInsFlag );
            }
            else
            {
                pNew = rColumn.CloneCell( i, nInsFlag, pDocument, ScAddress(nCol,nDestRow,nTab) );

                if ( pNew && pNew->GetNotePtr() && (nInsFlag & IDF_NOTE) == 0 )
                    pNew->DeleteNote();
            }

            if (pNew)
                Insert(nDestRow, pNew);
        }
    }
}


    //  Formelzellen werden jetzt schon hier kopiert,
    //  Notizen muessen aber evtl. noch geloescht werden

ScBaseCell* ScColumn::CloneCell(USHORT nIndex, USHORT nFlags,
                                    ScDocument* pDestDoc, const ScAddress& rDestPos)
{
    ScBaseCell* pNew = 0;
    ScBaseCell* pSource = pItems[nIndex].pCell;
    switch (pSource->GetCellType())
    {
        case CELLTYPE_NOTE:
            if (nFlags & IDF_NOTE)
                pNew = new ScNoteCell(*(ScNoteCell*)pSource);
            break;
        case CELLTYPE_EDIT:
            if (nFlags & IDF_STRING)
                pNew = new ScEditCell( *(ScEditCell*)pSource, pDestDoc );
            break;
        case CELLTYPE_STRING:
            if (nFlags & IDF_STRING)
                pNew = new ScStringCell(*(ScStringCell*)pSource);
            break;
        case CELLTYPE_VALUE:
            {
                BOOL bDoIns = FALSE;
                USHORT nMask = nFlags & ( IDF_DATETIME | IDF_VALUE );
                if ( nMask == (IDF_DATETIME | IDF_VALUE) )
                    bDoIns = TRUE;
                else if ( nMask )
                {
                    ULONG nNumIndex = (ULONG)((SfxUInt32Item*) GetAttr(
                            pItems[nIndex].nRow, ATTR_VALUE_FORMAT ))->GetValue();
                    short nTyp = pDocument->GetFormatTable()->GetType(nNumIndex);
                    if (nTyp == NUMBERFORMAT_DATE || nTyp == NUMBERFORMAT_TIME || nTyp == NUMBERFORMAT_DATETIME)
                        bDoIns = (nFlags & IDF_DATETIME)!=0;
                    else
                        bDoIns = (nFlags & IDF_VALUE)!=0;
                }
                if (bDoIns)
                    pNew = new ScValueCell(*(ScValueCell*)pSource);
            }
            break;
        case CELLTYPE_FORMULA:
            {
                ScFormulaCell* pForm = (ScFormulaCell*)pSource;
                if (nFlags & IDF_FORMULA)
                {
                    pNew = pForm->Clone( pDestDoc, rDestPos, TRUE );
                }
                else if ( (nFlags & (IDF_VALUE | IDF_DATETIME | IDF_STRING)) &&
                            !pDestDoc->IsUndo() )
                {
                    //  #48491# ins Undo-Dokument immer nur die Original-Zelle kopieren,
                    //  aus Formeln keine Value/String-Zellen erzeugen

                    USHORT nErr = pForm->GetErrCode();
                    if ( nErr )
                    {
                        //  Fehler werden immer mit "Zahlen" kopiert
                        //  (Das ist hiermit willkuerlich so festgelegt)

                        if ( nFlags & IDF_VALUE )
                        {
                            ScFormulaCell* pErrCell = new ScFormulaCell( pDestDoc, rDestPos );
                            pErrCell->SetErrCode( nErr );
                            pNew = pErrCell;
                        }
                    }
                    else if ( pForm->IsValue() )
                    {
                        BOOL bDoIns = FALSE;
                        USHORT nMask = nFlags & ( IDF_DATETIME | IDF_VALUE );
                        if ( nMask == (IDF_DATETIME | IDF_VALUE) )
                            bDoIns = TRUE;
                        else if ( nMask )
                        {
                            ULONG nNumIndex = (ULONG)((SfxUInt32Item*) GetAttr(
                                    pItems[nIndex].nRow, ATTR_VALUE_FORMAT ))->GetValue();
                            short nTyp = pDocument->GetFormatTable()->GetType(nNumIndex);
                            if (nTyp == NUMBERFORMAT_DATE || nTyp == NUMBERFORMAT_TIME || nTyp == NUMBERFORMAT_DATETIME)
                                bDoIns = (nFlags & IDF_DATETIME)!=0;
                            else
                                bDoIns = (nFlags & IDF_VALUE)!=0;
                        }

                        if (bDoIns)
                        {
                            double nVal = pForm->GetValue();
                            pNew = new ScValueCell(nVal);
                        }
                    }
                    else
                    {
                        if (nFlags & IDF_STRING)
                        {
                            String aString;
                            pForm->GetString(aString);
                            if ( aString.Len() )
                                pNew = new ScStringCell(aString);
                                // #33224# LeerStrings nicht kopieren
                        }
                    }
                    if ( pNew && pSource->GetNotePtr() && ( nFlags & IDF_NOTE ) )
                        pNew->SetNote(*pSource->GetNotePtr());
                }
            }
            break;
    }

    if ( !pNew && pSource->GetNotePtr() && ( nFlags & IDF_NOTE ) )
        pNew = new ScNoteCell(*pSource->GetNotePtr());

    return pNew;
}


void ScColumn::MixMarked( const ScMarkData& rMark, USHORT nFunction,
                            BOOL bSkipEmpty, ScColumn& rSrcCol )
{
    USHORT nRow1, nRow2;

    if (rMark.IsMultiMarked())
    {
        ScMarkArrayIter aIter( rMark.GetArray()+nCol );
        while (aIter.Next( nRow1, nRow2 ))
            MixData( nRow1, nRow2, nFunction, bSkipEmpty, rSrcCol );
    }
}


//  Ergebnis in rVal1

BOOL lcl_DoFunction( double& rVal1, double nVal2, USHORT nFunction )
{
    BOOL bOk = FALSE;
    switch (nFunction)
    {
        case PASTE_ADD:
            bOk = SubTotal::SafePlus( rVal1, nVal2 );
            break;
        case PASTE_SUB:
            nVal2 = -nVal2;     //! geht das immer ohne Fehler?
            bOk = SubTotal::SafePlus( rVal1, nVal2 );
            break;
        case PASTE_MUL:
            bOk = SubTotal::SafeMult( rVal1, nVal2 );
            break;
        case PASTE_DIV:
            bOk = SubTotal::SafeDiv( rVal1, nVal2 );
            break;
    }
    return bOk;
}


void lcl_AddCode( ScTokenArray& rArr, ScFormulaCell* pCell )
{
    rArr.AddOpCode(ocOpen);

    ScTokenArray* pCode = pCell->GetCode();
    if (pCode)
    {
        ScToken* pToken = pCode->First();
        while (pToken)
        {
            rArr.AddToken(*pToken);
            pToken = pCode->Next();
        }
    }

    rArr.AddOpCode(ocClose);
}


void ScColumn::MixData( USHORT nRow1, USHORT nRow2,
                            USHORT nFunction, BOOL bSkipEmpty,
                            ScColumn& rSrcCol )
{
    USHORT nSrcCount = rSrcCol.nCount;

    USHORT nIndex;
    Search( nRow1, nIndex );

//  USHORT nSrcIndex = 0;
    USHORT nSrcIndex;
    rSrcCol.Search( nRow1, nSrcIndex );         //! Testen, ob Daten ganz vorne

    USHORT nNextThis = MAXROW+1;
    if ( nIndex < nCount )
        nNextThis = pItems[nIndex].nRow;
    USHORT nNextSrc = MAXROW+1;
    if ( nSrcIndex < nSrcCount )
        nNextSrc = rSrcCol.pItems[nSrcIndex].nRow;

    while ( nNextThis <= nRow2 || nNextSrc <= nRow2 )
    {
        USHORT nRow = Min( nNextThis, nNextSrc );

        ScBaseCell* pSrc = NULL;
        ScBaseCell* pDest = NULL;
        ScBaseCell* pNew = NULL;
        BOOL bDelete = FALSE;

        if ( nSrcIndex < nSrcCount && nNextSrc == nRow )
            pSrc = rSrcCol.pItems[nSrcIndex].pCell;

        if ( nIndex < nCount && nNextThis == nRow )
            pDest = pItems[nIndex].pCell;

        DBG_ASSERT( pSrc || pDest, "Nanu ??!?" );

        CellType eSrcType  = pSrc  ? pSrc->GetCellType()  : CELLTYPE_NONE;
        CellType eDestType = pDest ? pDest->GetCellType() : CELLTYPE_NONE;

        BOOL bSrcEmpty = ( eSrcType == CELLTYPE_NONE || eSrcType == CELLTYPE_NOTE );
        BOOL bDestEmpty = ( eDestType == CELLTYPE_NONE || eDestType == CELLTYPE_NOTE );

        if ( bSkipEmpty && bDestEmpty )     // Originalzelle wiederherstellen
        {
            if ( pSrc )                     // war da eine Zelle?
            {
                pNew = pSrc->Clone( pDocument );
            }
        }
        else if ( nFunction )               // wirklich Rechenfunktion angegeben
        {
            double nVal1;
            double nVal2;
            if ( eSrcType == CELLTYPE_VALUE )
                nVal1 = ((ScValueCell*)pSrc)->GetValue();
            else
                nVal1 = 0.0;
            if ( eDestType == CELLTYPE_VALUE )
                nVal2 = ((ScValueCell*)pDest)->GetValue();
            else
                nVal2 = 0.0;

            //  leere Zellen werden als Werte behandelt

            BOOL bSrcVal  = ( bSrcEmpty || eSrcType == CELLTYPE_VALUE );
            BOOL bDestVal  = ( bDestEmpty || eDestType == CELLTYPE_VALUE );

            BOOL bSrcText = ( eSrcType == CELLTYPE_STRING ||
                                eSrcType == CELLTYPE_EDIT );
            BOOL bDestText = ( eDestType == CELLTYPE_STRING ||
                                eDestType == CELLTYPE_EDIT );

            //  sonst bleibt nur Formel...

            if ( bSrcEmpty && bDestEmpty )
            {
                //  beide leer -> nix
            }
            else if ( bSrcVal && bDestVal )
            {
                //  neuen Wert eintragen, oder Fehler bei Ueberlauf

                BOOL bOk = lcl_DoFunction( nVal1, nVal2, nFunction );

                if (bOk)
                    pNew = new ScValueCell( nVal1 );
                else
                {
                    ScFormulaCell* pFC = new ScFormulaCell( pDocument,
                                                ScAddress( nCol, nRow, nTab ) );
                    pFC->SetErrCode( errNoValue );
                    //! oder NOVALUE, dann auch in consoli,
                    //! sonst in Interpreter::GetCellValue die Abfrage auf errNoValue raus
                    //! (dann geht Stringzelle+Wertzelle nicht mehr)
                    pNew = pFC;
                }
            }
            else if ( bSrcText || bDestText )
            {
                //  mit Texten wird nicht gerechnet - immer "alte" Zelle, also pSrc

                if (pSrc)
                    pNew = pSrc->Clone( pDocument );
                else if (pDest)
                    bDelete = TRUE;
            }
            else
            {
                //  Kombination aus Wert und mindestens einer Formel -> Formel erzeugen

                ScTokenArray aArr;

                //  erste Zelle
                if ( eSrcType == CELLTYPE_FORMULA )
                    lcl_AddCode( aArr, (ScFormulaCell*)pSrc );
                else
                    aArr.AddDouble( nVal1 );

                //  Operator
                OpCode eOp = ocAdd;
                switch ( nFunction )
                {
                    case PASTE_ADD: eOp = ocAdd; break;
                    case PASTE_SUB: eOp = ocSub; break;
                    case PASTE_MUL: eOp = ocMul; break;
                    case PASTE_DIV: eOp = ocDiv; break;
                }
                aArr.AddOpCode(eOp);                // Funktion

                //  zweite Zelle
                if ( eDestType == CELLTYPE_FORMULA )
                    lcl_AddCode( aArr, (ScFormulaCell*)pDest );
                else
                    aArr.AddDouble( nVal2 );

                pNew = new ScFormulaCell( pDocument, ScAddress( nCol, nRow, nTab ), &aArr );
            }
        }


        if ( pNew || bDelete )          // neues Ergebnis ?
        {
            if (pDest && !pNew)                     // alte Zelle da ?
            {
                if ( pDest->GetBroadcaster() )
                    pNew = new ScNoteCell;          // Broadcaster uebernehmen
                else
                    Delete(nRow);                   // -> loeschen
            }
            if (pNew)
                Insert(nRow, pNew);     // neue einfuegen

            Search( nRow, nIndex );     // alles kann sich verschoben haben
            if (pNew)
                nNextThis = nRow;       // nIndex zeigt jetzt genau auf nRow
            else
                nNextThis = ( nIndex < nCount ) ? pItems[nIndex].nRow : MAXROW+1;
        }

        if ( nNextThis == nRow )
        {
            ++nIndex;
            nNextThis = ( nIndex < nCount ) ? pItems[nIndex].nRow : MAXROW+1;
        }
        if ( nNextSrc == nRow )
        {
            ++nSrcIndex;
            nNextSrc = ( nSrcIndex < nSrcCount ) ?
                            rSrcCol.pItems[nSrcIndex].nRow :
                            MAXROW+1;
        }
    }
}


ScAttrIterator* ScColumn::CreateAttrIterator( USHORT nStartRow, USHORT nEndRow ) const
{
    return new ScAttrIterator( pAttrArray, nStartRow, nEndRow );
}


void ScColumn::StartAllListeners()
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                USHORT nRow = pItems[i].nRow;
                ((ScFormulaCell*)pCell)->StartListeningTo( pDocument );
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener eingefuegt?
            }
        }
}


void ScColumn::StartRelNameListeners()
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                USHORT nRow = pItems[i].nRow;
                ((ScFormulaCell*)pCell)->StartListeningTo( pDocument, TRUE );
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener eingefuegt?
            }
        }
}


void ScColumn::BroadcastInArea( USHORT nRow1, USHORT nRow2 )
{
    if ( pItems )
    {
        USHORT nIndex, nRow;
        Search( nRow1, nIndex );
        while ( nIndex < nCount && (nRow = pItems[nIndex].nRow) <= nRow2 )
        {
            ScBaseCell* pCell = pItems[nIndex].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->SetDirty();
            else
                pDocument->Broadcast( SC_HINT_DATACHANGED,
                    ScAddress( nCol, nRow, nTab ), pCell );
            nIndex++;
        }
    }
}


void ScColumn::StartListeningInArea( USHORT nRow1, USHORT nRow2 )
{
    if ( pItems )
    {
        USHORT nIndex, nRow;
        Search( nRow1, nIndex );
        while ( nIndex < nCount && (nRow = pItems[nIndex].nRow) <= nRow2 )
        {
            ScBaseCell* pCell = pItems[nIndex].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->StartListeningTo( pDocument );
            if ( nRow != pItems[nIndex].nRow )
                Search( nRow, nIndex );     // durch Listening eingefuegt
            nIndex++;
        }
    }
}


//  TRUE = Zahlformat gesetzt
BOOL ScColumn::SetString( USHORT nRow, USHORT nTab, const String& rString )
{
    BOOL bNumFmtSet = FALSE;
    if (VALIDROW(nRow))
    {
        ScBaseCell* pNewCell = NULL;
        BOOL bIsLoading = FALSE;
        if (rString.Len() > 0)
        {
            double nVal;
            ULONG nIndex, nOldIndex;
            sal_Unicode cFirstChar;
            SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
            SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
            if ( pDocSh )
                bIsLoading = pDocSh->IsLoading();
            // IsLoading bei ConvertFrom Import
            if ( !bIsLoading )
            {
                nIndex = nOldIndex = GetNumberFormat( nRow );
                if ( rString.Len() > 1
                        && pFormatter->GetType(nIndex) != NUMBERFORMAT_TEXT )
                    cFirstChar = rString.GetChar(0);
                else
                    cFirstChar = 0;                             // Text
            }
            else
            {   // waehrend ConvertFrom Import gibt es keine gesetzten Formate
                cFirstChar = rString.GetChar(0);
            }

            if ( cFirstChar == '=' )
            {
                if ( rString.Len() == 1 )                       // = Text
                    pNewCell = new ScStringCell( rString );
                else                                            // =Formel
                    pNewCell = new ScFormulaCell( pDocument,
                        ScAddress( nCol, nRow, nTab ), rString, 0 );
            }
            else if ( cFirstChar == '\'')                       // 'Text
                pNewCell = new ScStringCell( rString.Copy(1) );
            else
            {
                BOOL bIsText = FALSE;
                if ( bIsLoading )
                {
                    if ( pItems && nCount )
                    {
                        String aStr;
                        USHORT i = nCount;
                        USHORT nStop = (i >= 3 ? i - 3 : 0);
                        // die letzten Zellen vergleichen, ob gleicher String
                        // und IsNumberFormat eingespart werden kann
                        do
                        {
                            i--;
                            ScBaseCell* pCell = pItems[i].pCell;
                            switch ( pCell->GetCellType() )
                            {
                                case CELLTYPE_STRING :
                                    ((ScStringCell*)pCell)->GetString( aStr );
                                    if ( rString == aStr )
                                        bIsText = TRUE;
                                break;
                                case CELLTYPE_NOTE :    // durch =Formel referenziert
                                break;
                                default:
                                    if ( i == nCount - 1 )
                                        i = 0;
                                        // wahrscheinlich ganze Spalte kein String
                            }
                        } while ( i && i > nStop && !bIsText );
                    }
                    // nIndex fuer IsNumberFormat vorbelegen
                    if ( !bIsText )
                        nIndex = nOldIndex = pFormatter->GetStandardIndex();
                }
                if ( !bIsText &&
                        pFormatter->IsNumberFormat(rString, nIndex, nVal) )
                {                                               // Zahl
                    pNewCell = new ScValueCell( nVal );
                    if ( nIndex != nOldIndex)
                    {
                        ApplyAttr( nRow, SfxUInt32Item( ATTR_VALUE_FORMAT,
                            (UINT32) nIndex) );
                        bNumFmtSet = TRUE;
                    }
                }
                else                                            // Text
                    pNewCell = new ScStringCell( rString );
            }
        }

        if ( bIsLoading && (!nCount || nRow > pItems[nCount-1].nRow) )
        {   // Search einsparen und ohne Umweg ueber Insert, Listener aufbauen
            // und Broadcast kommt eh erst nach dem Laden
            if ( pNewCell )
                Append( nRow, pNewCell );
        }
        else
        {
            USHORT i;
            if (Search(nRow, i))
            {
                ScBaseCell* pOldCell = pItems[i].pCell;
                const ScPostIt* pNote = pOldCell->GetNotePtr();
                ScBroadcasterList* pBC = pOldCell->GetBroadcaster();
                if (pNewCell || pNote || pBC)
                {
                    if (!pNewCell)
                        pNewCell = new ScNoteCell();
                    if (pNote)
                        pNewCell->SetNote(*pNote);
                    if (pBC)
                    {
                        pNewCell->SetBroadcaster(pBC);
                        pOldCell->ForgetBroadcaster();
                        pLastFormulaTreeTop = 0;    // Err527 Workaround
                    }

                    if ( pOldCell->GetCellType() == CELLTYPE_FORMULA )
                    {
                        pOldCell->EndListeningTo( pDocument );
                        // falls in EndListening NoteCell in gleicher Col zerstoert
                        if ( i >= nCount || pItems[i].nRow != nRow )
                            Search(nRow, i);
                    }
                    pOldCell->Delete();
                    pItems[i].pCell = pNewCell;         // ersetzen
                    if ( pNewCell->GetCellType() == CELLTYPE_FORMULA )
                    {
                        pNewCell->StartListeningTo( pDocument );
                        ((ScFormulaCell*)pNewCell)->SetDirty();
                    }
                    else
                        pDocument->Broadcast( SC_HINT_DATACHANGED,
                            ScAddress( nCol, nRow, nTab ), pNewCell );
                }
                else
                {
                    DeleteAtIndex(i);                   // loeschen und Broadcast
                }
            }
            else if (pNewCell)
            {
                Insert(nRow, pNewCell);                 // neu eintragen und Broadcast
            }
        }

        //  hier keine Formate mehr fuer Formeln setzen!
        //  (werden bei der Ausgabe abgefragt)

    }
    return bNumFmtSet;
}


void ScColumn::GetFilterEntries(USHORT nStartRow, USHORT nEndRow, TypedStrCollection& rStrings)
{
    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
    String aString;
    USHORT nRow;
    USHORT nIndex;

    Search( nStartRow, nIndex );

    while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEndRow) : FALSE )
    {
        ScBaseCell*          pCell    = pItems[nIndex].pCell;
        TypedStrData*        pData;
        ULONG                nFormat  = GetNumberFormat( nRow );

        ScCellFormat::GetInputString( pCell, nFormat, aString, *pFormatter );

        if ( pDocument->HasStringData( nCol, nRow, nTab ) )
            pData = new TypedStrData( aString );
        else
        {
            double nValue;

            switch ( pCell->GetCellType() )
            {
                case CELLTYPE_VALUE:
                    nValue = ((ScValueCell*)pCell)->GetValue();
                    break;

                case CELLTYPE_FORMULA:
                    nValue = ((ScFormulaCell*)pCell)->GetValue();
                    break;

                default:
                    nValue = 0.0;
            }

            pData = new TypedStrData( aString, nValue, SC_STRTYPE_VALUE );
        }

        if ( !rStrings.Insert( pData ) )
            delete pData;                               // doppelt

        ++nIndex;
    }
}

//
//  GetDataEntries - Strings aus zusammenhaengendem Bereich um nRow
//

//  DATENT_MAX      - max. Anzahl Eintrage in Liste fuer Auto-Eingabe
//  DATENT_SEARCH   - max. Anzahl Zellen, die durchsucht werden - neu: nur Strings zaehlen
#define DATENT_MAX      200
#define DATENT_SEARCH   2000


BOOL ScColumn::GetDataEntries(USHORT nStartRow, TypedStrCollection& rStrings, BOOL bLimit)
{
    BOOL bFound = FALSE;
    USHORT nThisIndex;
    BOOL bThisUsed = Search( nStartRow, nThisIndex );
    String aString;
    USHORT nCells = 0;

    //  Die Beschraenkung auf angrenzende Zellen (ohne Luecken) ist nicht mehr gewollt
    //  (Featurekommission zur 5.1), stattdessen abwechselnd nach oben und unten suchen,
    //  damit naheliegende Zellen wenigstens zuerst gefunden werden.
    //! Abstaende der Zeilennummern vergleichen? (Performance??)

    USHORT nUpIndex = nThisIndex;       // zeigt hinter die Zelle
    USHORT nDownIndex = nThisIndex;     // zeigt auf die Zelle
    if (bThisUsed)
        ++nDownIndex;                   // Startzelle ueberspringen

    while ( nUpIndex || nDownIndex < nCount )
    {
        if ( nUpIndex )                 // nach oben
        {
            ScBaseCell* pCell = pItems[nUpIndex-1].pCell;
            CellType eType = pCell->GetCellType();
            if (eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT)     // nur Strings interessieren
            {
                if (eType == CELLTYPE_STRING)
                    ((ScStringCell*)pCell)->GetString(aString);
                else
                    ((ScEditCell*)pCell)->GetString(aString);

                TypedStrData* pData = new TypedStrData(aString);
                if ( !rStrings.Insert( pData ) )
                    delete pData;                                           // doppelt
                else if ( bLimit && rStrings.GetCount() >= DATENT_MAX )
                    break;                                                  // Maximum erreicht
                bFound = TRUE;

                if ( bLimit )
                    if (++nCells >= DATENT_SEARCH)
                        break;                                  // genug gesucht
            }
            --nUpIndex;
        }

        if ( nDownIndex < nCount )      // nach unten
        {
            ScBaseCell* pCell = pItems[nDownIndex].pCell;
            CellType eType = pCell->GetCellType();
            if (eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT)     // nur Strings interessieren
            {
                if (eType == CELLTYPE_STRING)
                    ((ScStringCell*)pCell)->GetString(aString);
                else
                    ((ScEditCell*)pCell)->GetString(aString);

                TypedStrData* pData = new TypedStrData(aString);
                if ( !rStrings.Insert( pData ) )
                    delete pData;                                           // doppelt
                else if ( bLimit && rStrings.GetCount() >= DATENT_MAX )
                    break;                                                  // Maximum erreicht
                bFound = TRUE;

                if ( bLimit )
                    if (++nCells >= DATENT_SEARCH)
                        break;                                  // genug gesucht
            }
            ++nDownIndex;
        }
    }

    return bFound;
}

#undef DATENT_MAX
#undef DATENT_SEARCH


void ScColumn::RemoveProtected( USHORT nStartRow, USHORT nEndRow )
{
    ScAttrIterator aAttrIter( pAttrArray, nStartRow, nEndRow );
    USHORT nTop;
    USHORT nBottom;
    USHORT nIndex;
    const ScPatternAttr* pPattern = aAttrIter.Next( nTop, nBottom );
    while (pPattern)
    {
        const ScProtectionAttr* pAttr = (const ScProtectionAttr*)&pPattern->GetItem(ATTR_PROTECTION);
        if ( pAttr->GetHideCell() )
            DeleteArea( nTop, nBottom, IDF_CONTENTS );
        else if ( pAttr->GetHideFormula() )
        {
            Search( nTop, nIndex );
            while ( nIndex<nCount && pItems[nIndex].nRow<=nBottom )
            {
                if ( pItems[nIndex].pCell->GetCellType() == CELLTYPE_FORMULA )
                {
                    ScFormulaCell* pFormula = (ScFormulaCell*)pItems[nIndex].pCell;
                    if (pFormula->IsValue())
                    {
                        double nVal = pFormula->GetValue();
                        pItems[nIndex].pCell = new ScValueCell( nVal );
                    }
                    else
                    {
                        String aString;
                        pFormula->GetString(aString);
                        pItems[nIndex].pCell = new ScStringCell( aString );
                    }
                    delete pFormula;
                }
                ++nIndex;
            }
        }

        pPattern = aAttrIter.Next( nTop, nBottom );
    }
}


void ScColumn::SetError( USHORT nRow, const USHORT nError)
{
    if (VALIDROW(nRow))
    {
        ScFormulaCell* pCell = new ScFormulaCell
            ( pDocument, ScAddress( nCol, nRow, nTab ) );
        pCell->SetErrCode( nError );
        Insert( nRow, pCell );
    }
}


void ScColumn::SetValue( USHORT nRow, const double& rVal)
{
    if (VALIDROW(nRow))
    {
        ScBaseCell* pCell = new ScValueCell(rVal);
        Insert( nRow, pCell );
    }
}


void ScColumn::SetNote( USHORT nRow, const ScPostIt& rNote)
{
    BOOL bEmpty = !rNote.GetText().Len();

    USHORT nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        if (bEmpty && pCell->GetCellType() == CELLTYPE_NOTE && !pCell->GetBroadcaster())
            DeleteAtIndex(nIndex);
        else
            pCell->SetNote(rNote);
    }
    else
    {
        if (!bEmpty)
            Insert(nRow, new ScNoteCell(rNote));
    }
}


void ScColumn::GetString( USHORT nRow, String& rString ) const
{
    USHORT  nIndex;
    Color* pColor;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        if (pCell->GetCellType() != CELLTYPE_NOTE)
        {
            ULONG nFormat = GetNumberFormat( nRow );
            ScCellFormat::GetString( pCell, nFormat, rString, &pColor, *(pDocument->GetFormatTable()) );
        }
        else
            rString.Erase();
    }
    else
        rString.Erase();
}


void ScColumn::GetInputString( USHORT nRow, String& rString ) const
{
    USHORT  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        if (pCell->GetCellType() != CELLTYPE_NOTE)
        {
            ULONG nFormat = GetNumberFormat( nRow );
            ScCellFormat::GetInputString( pCell, nFormat, rString, *(pDocument->GetFormatTable()) );
        }
        else
            rString.Erase();
    }
    else
        rString.Erase();
}


double ScColumn::GetValue( USHORT nRow ) const
{
    USHORT  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        switch (pCell->GetCellType())
        {
            case CELLTYPE_VALUE:
                return ((ScValueCell*)pCell)->GetValue();
                break;
            case CELLTYPE_FORMULA:
                {
                    if (((ScFormulaCell*)pCell)->IsValue())
                        return ((ScFormulaCell*)pCell)->GetValue();
                    else
                        return 0.0;
                }
                break;
            default:
                return 0.0;
                break;
        }
    }
    return 0.0;
}


void ScColumn::GetFormula( USHORT nRow, String& rFormula, BOOL ) const
{
    USHORT  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
            ((ScFormulaCell*)pCell)->GetFormula( rFormula );
        else
            rFormula.Erase();
    }
    else
        rFormula.Erase();
}


BOOL ScColumn::GetNote( USHORT nRow, ScPostIt& rNote) const
{
    BOOL    bHasNote = FALSE;
    USHORT  nIndex;
    if (Search(nRow, nIndex))
        bHasNote = pItems[nIndex].pCell->GetNote(rNote);
    else
        rNote.Clear();

    return bHasNote;
}


CellType ScColumn::GetCellType( USHORT nRow ) const
{
    USHORT  nIndex;
    if (Search(nRow, nIndex))
        return pItems[nIndex].pCell->GetCellType();
    return CELLTYPE_NONE;
}


USHORT ScColumn::GetErrCode( USHORT nRow ) const
{
    USHORT  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
            return ((ScFormulaCell*)pCell)->GetErrCode();
    }
    return 0;
}


BOOL ScColumn::HasStringData( USHORT nRow ) const
{
    USHORT  nIndex;
    if (Search(nRow, nIndex))
        return (pItems[nIndex].pCell)->HasStringData();
    return FALSE;
}


BOOL ScColumn::HasValueData( USHORT nRow ) const
{
    USHORT  nIndex;
    if (Search(nRow, nIndex))
        return (pItems[nIndex].pCell)->HasValueData();
    return FALSE;
}

BOOL ScColumn::HasStringCells( USHORT nStartRow, USHORT nEndRow ) const
{
    //  TRUE, wenn String- oder Editzellen im Bereich

    if ( pItems )
    {
        USHORT nIndex;
        Search( nStartRow, nIndex );
        while ( nIndex < nCount && pItems[nIndex].nRow <= nEndRow )
        {
            CellType eType = pItems[nIndex].pCell->GetCellType();
            if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
                return TRUE;
            ++nIndex;
        }
    }
    return FALSE;
}


xub_StrLen ScColumn::GetMaxStringLen( USHORT nRowStart, USHORT nRowEnd ) const
{
    xub_StrLen nStringLen = 0;
    if ( pItems )
    {
        String aString;
        SvNumberFormatter* pNumFmt = pDocument->GetFormatTable();
        USHORT nIndex, nRow;
        Search( nRowStart, nIndex );
        while ( nIndex < nCount && (nRow = pItems[nIndex].nRow) <= nRowEnd )
        {
            ScBaseCell* pCell = pItems[nIndex].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE )
            {
                Color* pColor;
                ULONG nFormat = (ULONG) ((SfxUInt32Item*) GetAttr(
                    nRow, ATTR_VALUE_FORMAT ))->GetValue();
                ScCellFormat::GetString( pCell, nFormat, aString, &pColor,
                    *pNumFmt );
                if ( nStringLen < aString.Len() )
                    nStringLen = aString.Len();
            }
            nIndex++;
        }
    }
    return nStringLen;
}


xub_StrLen ScColumn::GetMaxNumberStringLen( USHORT& nPrecision,
        USHORT nRowStart, USHORT nRowEnd ) const
{
    xub_StrLen nStringLen = 0;
    nPrecision = pDocument->GetDocOptions().GetStdPrecision();
    if ( pItems )
    {
        String aString;
        SvNumberFormatter* pNumFmt = pDocument->GetFormatTable();
        USHORT nIndex, nRow;
        Search( nRowStart, nIndex );
        while ( nIndex < nCount && (nRow = pItems[nIndex].nRow) <= nRowEnd )
        {
            ScBaseCell* pCell = pItems[nIndex].pCell;
            CellType eType = pCell->GetCellType();
            if ( eType == CELLTYPE_VALUE || (eType == CELLTYPE_FORMULA
                    && ((ScFormulaCell*)pCell)->IsValue()) )
            {
                ULONG nFormat = (ULONG) ((SfxUInt32Item*) GetAttr(
                    nRow, ATTR_VALUE_FORMAT ))->GetValue();
                ScCellFormat::GetInputString( pCell, nFormat, aString, *pNumFmt );
                xub_StrLen nLen = aString.Len();
                if ( nLen )
                {
                    if ( nFormat )
                    {   // more decimals than standard?
                        USHORT nPrec = pNumFmt->GetFormatPrecision( nFormat );
                        if ( nPrec > nPrecision )
                            nPrecision = nPrec;
                    }
                    if ( nPrecision )
                    {   // less than nPrecision in string => widen it
                        // more => shorten it
                        String aSep = pNumFmt->GetFormatDecimalSep( nFormat );
                        xub_StrLen nTmp = aString.Search( aSep );
                        if ( nTmp == STRING_NOTFOUND )
                            nLen += nPrecision + aSep.Len();
                        else
                        {
                            nTmp = aString.Len() - (nTmp + aSep.Len());
                            if ( nTmp != nPrecision )
                                nLen += nPrecision - nTmp;
                                // nPrecision > nTmp : nLen + Diff
                                // nPrecision < nTmp : nLen - Diff
                        }
                    }
                    if ( nStringLen < nLen )
                        nStringLen = nLen;
                }
            }
            nIndex++;
        }
    }
    return nStringLen;
}

