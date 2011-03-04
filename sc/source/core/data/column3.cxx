/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
// INCLUDE ---------------------------------------------------------------



#include <sfx2/objsh.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>

#include "scitems.hxx"
#include "column.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "collect.hxx"
#include "formula/errorcodes.hxx"
#include "formula/token.hxx"
#include "brdcst.hxx"
#include "docoptio.hxx"         // GetStdPrecision fuer GetMaxNumberStringLen
#include "subtotal.hxx"
#include "markdata.hxx"
#include "detfunc.hxx"          // fuer Notizen bei DeleteRange
#include "postit.hxx"
#include "stringutil.hxx"
#include "docpool.hxx"

#include <com/sun/star/i18n/LocaleDataItem.hpp>

using ::com::sun::star::i18n::LocaleDataItem;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// Err527 Workaround
extern const ScFormulaCell* pLastFormulaTreeTop;    // in cellform.cxx
using namespace formula;
// STATIC DATA -----------------------------------------------------------

bool ScColumn::bDoubleAlloc = FALSE;    // fuer Import: Groesse beim Allozieren verdoppeln


void ScColumn::Insert( SCROW nRow, ScBaseCell* pNewCell )
{
    BOOL bIsAppended = FALSE;
    if (pItems && nCount>0)
    {
        if (pItems[nCount-1].nRow < nRow)
        {
            Append(nRow, pNewCell );
            bIsAppended = TRUE;
        }
    }
    if ( !bIsAppended )
    {
        SCSIZE  nIndex;
        if (Search(nRow, nIndex))
        {
            ScBaseCell* pOldCell = pItems[nIndex].pCell;

            // move broadcaster and note to new cell, if not existing in new cell
            if (pOldCell->HasBroadcaster() && !pNewCell->HasBroadcaster())
                pNewCell->TakeBroadcaster( pOldCell->ReleaseBroadcaster() );
            if (pOldCell->HasNote() && !pNewCell->HasNote())
                pNewCell->TakeNote( pOldCell->ReleaseNote() );

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
                        if ( nLimit > sal::static_int_cast<SCSIZE>(MAXROWCOUNT) )
                            nLimit = MAXROWCOUNT;
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
                pDocument->Broadcast( ScHint( SC_HINT_DATACHANGED,
                    ScAddress( nCol, nRow, nTab ), pNewCell ) );
        }
    }
}


void ScColumn::Insert( SCROW nRow, sal_uInt32 nNumberFormat, ScBaseCell* pCell )
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


void ScColumn::Append( SCROW nRow, ScBaseCell* pCell )
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
                if ( nLimit > sal::static_int_cast<SCSIZE>(MAXROWCOUNT) )
                    nLimit = MAXROWCOUNT;
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


void ScColumn::Delete( SCROW nRow )
{
    SCSIZE  nIndex;

    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        ScNoteCell* pNoteCell = new ScNoteCell;
        pItems[nIndex].pCell = pNoteCell;       // Dummy fuer Interpret
        pDocument->Broadcast( ScHint( SC_HINT_DYING,
            ScAddress( nCol, nRow, nTab ), pCell ) );
        if ( SvtBroadcaster* pBC = pCell->ReleaseBroadcaster() )
        {
            pNoteCell->TakeBroadcaster( pBC );
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


void ScColumn::DeleteAtIndex( SCSIZE nIndex )
{
    ScBaseCell* pCell = pItems[nIndex].pCell;
    ScNoteCell* pNoteCell = new ScNoteCell;
    pItems[nIndex].pCell = pNoteCell;       // Dummy fuer Interpret
    pDocument->Broadcast( ScHint( SC_HINT_DYING,
        ScAddress( nCol, pItems[nIndex].nRow, nTab ), pCell ) );
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
        for (SCSIZE i = 0; i < nCount; i++)
            pItems[i].pCell->Delete();
        delete[] pItems;
        pItems = NULL;
    }
    nCount = 0;
    nLimit = 0;
}


void ScColumn::DeleteRow( SCROW nStartRow, SCSIZE nSize )
{
    pAttrArray->DeleteRow( nStartRow, nSize );

    if ( !pItems || !nCount )
        return ;

    SCSIZE nFirstIndex;
    Search( nStartRow, nFirstIndex );
    if ( nFirstIndex >= nCount )
        return ;

    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden

    BOOL bFound=FALSE;
    SCROW nEndRow = nStartRow + nSize - 1;
    SCSIZE nStartIndex = 0;
    SCSIZE nEndIndex = 0;
    SCSIZE i;

    for ( i = nFirstIndex; i < nCount && pItems[i].nRow <= nEndRow; i++ )
    {
        if (!bFound)
        {
            nStartIndex = i;
            bFound = TRUE;
        }
        nEndIndex = i;

        ScBaseCell* pCell = pItems[i].pCell;
        SvtBroadcaster* pBC = pCell->GetBroadcaster();
        if (pBC)
        {
// gibt jetzt invalid reference, kein Aufruecken der direkten Referenzen
//          MoveListeners( *pBC, nRow+nSize );
            pCell->DeleteBroadcaster();
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

    ScAddress aAdr( nCol, 0, nTab );
    ScHint aHint( SC_HINT_DATACHANGED, aAdr, NULL );    // only areas (ScBaseCell* == NULL)
    ScAddress& rAddress = aHint.GetAddress();
    // for sparse occupation use single broadcasts, not ranges
    BOOL bSingleBroadcasts = (((pItems[nCount-1].nRow - pItems[i].nRow) /
                (nCount - i)) > 1);
    if ( bSingleBroadcasts )
    {
        SCROW nLastBroadcast = MAXROW+1;
        for ( ; i < nCount; i++ )
        {
            SCROW nOldRow = pItems[i].nRow;
            // Aenderung Quelle broadcasten
            rAddress.SetRow( nOldRow );
            pDocument->AreaBroadcast( aHint );
            SCROW nNewRow = (pItems[i].nRow -= nSize);
            // Aenderung Ziel broadcasten
            if ( nLastBroadcast != nNewRow )
            {   // direkt aufeinanderfolgende nicht doppelt broadcasten
                rAddress.SetRow( nNewRow );
                pDocument->AreaBroadcast( aHint );
            }
            nLastBroadcast = nOldRow;
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
        }
    }
    else
    {
        rAddress.SetRow( pItems[i].nRow );
        ScRange aRange( rAddress );
        aRange.aEnd.SetRow( pItems[nCount-1].nRow );
        for ( ; i < nCount; i++ )
        {
            SCROW nNewRow = (pItems[i].nRow -= nSize);
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
        }
        pDocument->AreaBroadcastInRange( aRange, aHint );
    }

    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::DeleteRange( SCSIZE nStartIndex, SCSIZE nEndIndex, USHORT nDelFlag )
{
    /*  If caller specifies to not remove the note caption objects, all cells
        have to forget the pointers to them. This is used e.g. while undoing a
        "paste cells" operation, which removes the caption objects later in
        drawing undo. */
    bool bDeleteNote = (nDelFlag & IDF_NOTE) != 0;
    bool bNoCaptions = (nDelFlag & IDF_NOCAPTIONS) != 0;
    if (bDeleteNote && bNoCaptions)
        for ( SCSIZE nIdx = nStartIndex; nIdx <= nEndIndex; ++nIdx )
            if ( ScPostIt* pNote = pItems[ nIdx ].pCell->GetNote() )
                pNote->ForgetCaption();

    // special simple mode if all contents are deleted and cells do not contain broadcasters
    bool bSimple = ((nDelFlag & IDF_CONTENTS) == IDF_CONTENTS);
    if (bSimple)
        for ( SCSIZE nIdx = nStartIndex; bSimple && (nIdx <= nEndIndex); ++nIdx )
            if (pItems[ nIdx ].pCell->GetBroadcaster())
                bSimple = false;

    ScHint aHint( SC_HINT_DYING, ScAddress( nCol, 0, nTab ), 0 );

    // cache all formula cells, they will be deleted at end of this function
    typedef ::std::vector< ScFormulaCell* > FormulaCellVector;
    FormulaCellVector aDelCells;
    aDelCells.reserve( nEndIndex - nStartIndex + 1 );

    // simple deletion of the cell objects
    if (bSimple)
    {
        // pNoteCell: dummy replacement for old cells, to prevent that interpreter uses old cell
        ScNoteCell* pNoteCell = new ScNoteCell;
        for ( SCSIZE nIdx = nStartIndex; nIdx <= nEndIndex; ++nIdx )
        {
            ScBaseCell* pOldCell = pItems[ nIdx ].pCell;
            if (pOldCell->GetCellType() == CELLTYPE_FORMULA)
            {
                // cache formula cell, will be deleted below
                aDelCells.push_back( static_cast< ScFormulaCell* >( pOldCell ) );
            }
            else
            {
                // interpret in broadcast must not use the old cell
                pItems[ nIdx ].pCell = pNoteCell;
                aHint.GetAddress().SetRow( pItems[ nIdx ].nRow );
                aHint.SetCell( pOldCell );
                pDocument->Broadcast( aHint );
                pOldCell->Delete();
            }
        }
        delete pNoteCell;
        memmove( &pItems[nStartIndex], &pItems[nEndIndex + 1], (nCount - nEndIndex - 1) * sizeof(ColEntry) );
        nCount -= nEndIndex-nStartIndex+1;
    }

    // else: delete some contents of the cells
    else
    {
        SCSIZE j = nStartIndex;
        for ( SCSIZE nIdx = nStartIndex; nIdx <= nEndIndex; ++nIdx )
        {
            // decide whether to delete the cell object according to passed flags
            bool bDelete = false;
            ScBaseCell* pOldCell = pItems[j].pCell;
            CellType eCellType = pOldCell->GetCellType();
            switch ( eCellType )
            {
                case CELLTYPE_VALUE:
                {
                    USHORT nValFlags = nDelFlag & (IDF_DATETIME|IDF_VALUE);
                    // delete values and dates?
                    bDelete = nValFlags == (IDF_DATETIME|IDF_VALUE);
                    // if not, decide according to cell number format
                    if( !bDelete && (nValFlags != 0) )
                    {
                        ULONG nIndex = (ULONG)((SfxUInt32Item*)GetAttr( pItems[j].nRow, ATTR_VALUE_FORMAT ))->GetValue();
                        short nType = pDocument->GetFormatTable()->GetType(nIndex);
                        bool bIsDate = (nType == NUMBERFORMAT_DATE) || (nType == NUMBERFORMAT_TIME) || (nType == NUMBERFORMAT_DATETIME);
                        bDelete = nValFlags == (bIsDate ? IDF_DATETIME : IDF_VALUE);
                    }
                }
                break;

                case CELLTYPE_STRING:
                case CELLTYPE_EDIT:
                    bDelete = (nDelFlag & IDF_STRING) != 0;
                break;

                case CELLTYPE_FORMULA:
                    bDelete = (nDelFlag & IDF_FORMULA) != 0;
                break;

                case CELLTYPE_NOTE:
                    // do note delete note cell with broadcaster
                    bDelete = bDeleteNote && !pOldCell->GetBroadcaster();
                break;

                default:;   // added to avoid warnings
            }

            if (bDelete)
            {
                // try to create a replacement note cell, if note or broadcaster exists
                ScNoteCell* pNoteCell = 0;
                if (eCellType != CELLTYPE_NOTE)
                {
                    // do not rescue note if it has to be deleted according to passed flags
                    ScPostIt* pNote = bDeleteNote ? 0 : pOldCell->ReleaseNote();
                    // #i99844# do not release broadcaster from old cell, it still has to notify deleted content
                    SvtBroadcaster* pBC = pOldCell->GetBroadcaster();
                    if( pNote || pBC )
                        pNoteCell = new ScNoteCell( pNote, pBC );
                }

                // remove cell entry in cell item list
                SCROW nOldRow = pItems[j].nRow;
                if (pNoteCell)
                {
                    // replace old cell with the replacement note cell
                    pItems[j].pCell = pNoteCell;
                    ++j;
                }
                else
                {
                    // remove the old cell from the cell item list
                    --nCount;
                    memmove( &pItems[j], &pItems[j + 1], (nCount - j) * sizeof(ColEntry) );
                    pItems[nCount].nRow = 0;
                    pItems[nCount].pCell = 0;
                }

                // cache formula cells (will be deleted later), delete cell of other type
                if (eCellType == CELLTYPE_FORMULA)
                {
                    aDelCells.push_back( static_cast< ScFormulaCell* >( pOldCell ) );
                }
                else
                {
                    aHint.GetAddress().SetRow( nOldRow );
                    aHint.SetCell( pOldCell );
                    pDocument->Broadcast( aHint );
                    // #i99844# after broadcasting, old cell has to forget the broadcaster (owned by pNoteCell)
                    pOldCell->ReleaseBroadcaster();
                    pOldCell->Delete();
                }
            }
            else
            {
                // delete cell note
                if (bDeleteNote)
                    pItems[j].pCell->DeleteNote();
                // cell not deleted, move index to next cell
                ++j;
            }
        }
    }

    // *** delete all formula cells ***

    // first, all cells stop listening, may save unneeded recalcualtions
    for ( FormulaCellVector::iterator aIt = aDelCells.begin(), aEnd = aDelCells.end(); aIt != aEnd; ++aIt )
        (*aIt)->EndListeningTo( pDocument );

    // #i101869# if the note cell with the broadcaster was deleted in EndListening,
    // forget the pointer to the broadcaster
    for ( FormulaCellVector::iterator aIt = aDelCells.begin(), aEnd = aDelCells.end(); aIt != aEnd; ++aIt )
    {
        SCSIZE nIndex;
        if ( !Search( (*aIt)->aPos.Row(), nIndex ) )
            (*aIt)->ReleaseBroadcaster();
    }

    // broadcast SC_HINT_DYING for all cells and delete them
    for ( FormulaCellVector::iterator aIt = aDelCells.begin(), aEnd = aDelCells.end(); aIt != aEnd; ++aIt )
    {
        aHint.SetAddress( (*aIt)->aPos );
        aHint.SetCell( *aIt );
        pDocument->Broadcast( aHint );
        // #i99844# after broadcasting, old cell has to forget the broadcaster (owned by replacement note cell)
        (*aIt)->ReleaseBroadcaster();
        (*aIt)->Delete();
    }
}


void ScColumn::DeleteArea(SCROW nStartRow, SCROW nEndRow, USHORT nDelFlag)
{
    //  FreeAll darf hier nicht gerufen werden wegen Broadcastern

    //  Attribute erst am Ende, damit vorher noch zwischen Zahlen und Datum
    //  unterschieden werden kann (#47901#)

    USHORT nContMask = IDF_CONTENTS;
    //  IDF_NOCAPTIONS needs to be passed too, if IDF_NOTE is set
    if( nDelFlag & IDF_NOTE )
        nContMask |= IDF_NOCAPTIONS;
    USHORT nContFlag = nDelFlag & nContMask;

    if (pItems && nCount>0 && nContFlag)
    {
        if (nStartRow==0 && nEndRow==MAXROW)
            DeleteRange( 0, nCount-1, nContFlag );
        else
        {
            BOOL bFound=FALSE;
            SCSIZE nStartIndex = 0;
            SCSIZE nEndIndex = 0;
            for (SCSIZE i = 0; i < nCount; i++)
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
                                            SCSIZE nIndex, USHORT nFlags ) const
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
        default:
        {
            // added to avoid warnings
        }
    }
    if (!bMatch)
        return NULL;


    //  Referenz einsetzen
    ScSingleRefData aRef;
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

void ScColumn::CopyFromClip(SCROW nRow1, SCROW nRow2, long nDy,
                                USHORT nInsFlag, bool bAsLink, bool bSkipAttrForEmpty,
                                ScColumn& rColumn)
{
    if ((nInsFlag & IDF_ATTRIB) != 0)
    {
        if ( bSkipAttrForEmpty )
        {
            //  copy only attributes for non-empty cells
            //  (notes are not counted as non-empty here, to match the content behavior)

            SCSIZE nStartIndex;
            rColumn.Search( nRow1-nDy, nStartIndex );
            while ( nStartIndex < rColumn.nCount && rColumn.pItems[nStartIndex].nRow <= nRow2-nDy )
            {
                SCSIZE nEndIndex = nStartIndex;
                if ( rColumn.pItems[nStartIndex].pCell->GetCellType() != CELLTYPE_NOTE )
                {
                    SCROW nStartRow = rColumn.pItems[nStartIndex].nRow;
                    SCROW nEndRow = nStartRow;

                    //  find consecutive non-empty cells

                    while ( nEndRow < nRow2-nDy &&
                            nEndIndex+1 < rColumn.nCount &&
                            rColumn.pItems[nEndIndex+1].nRow == nEndRow+1 &&
                            rColumn.pItems[nEndIndex+1].pCell->GetCellType() != CELLTYPE_NOTE )
                    {
                        ++nEndIndex;
                        ++nEndRow;
                    }

                    rColumn.pAttrArray->CopyAreaSafe( nStartRow+nDy, nEndRow+nDy, nDy, *pAttrArray );
                }
                nStartIndex = nEndIndex + 1;
            }
        }
        else
            rColumn.pAttrArray->CopyAreaSafe( nRow1, nRow2, nDy, *pAttrArray );
    }
    if ((nInsFlag & IDF_CONTENTS) == 0)
        return;

    if ( bAsLink && nInsFlag == IDF_ALL )
    {
        //  bei "alles" werden auch leere Zellen referenziert
        //! IDF_ALL muss immer mehr Flags enthalten, als bei "Inhalte Einfuegen"
        //! einzeln ausgewaehlt werden koennen!

        Resize( nCount + static_cast<SCSIZE>(nRow2-nRow1+1) );

        ScAddress aDestPos( nCol, 0, nTab );        // Row wird angepasst

        //  Referenz erzeugen (Quell-Position)
        ScSingleRefData aRef;
        aRef.nCol = rColumn.nCol;
        //  nRow wird angepasst
        aRef.nTab = rColumn.nTab;
        aRef.InitFlags();                           // -> alles absolut
        aRef.SetFlag3D(TRUE);

        for (SCROW nDestRow = nRow1; nDestRow <= nRow2; nDestRow++)
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

    SCSIZE nColCount = rColumn.nCount;

    // ignore IDF_FORMULA - "all contents but no formulas" results in the same number of cells
    if ((nInsFlag & ( IDF_CONTENTS & ~IDF_FORMULA )) == ( IDF_CONTENTS & ~IDF_FORMULA ) && nRow2-nRow1 >= 64)
    {
        //! Always do the Resize from the outside, where the number of repetitions is known
        //! (then it can be removed here)

        SCSIZE nNew = nCount + nColCount;
        if ( nLimit < nNew )
            Resize( nNew );
    }

    // IDF_ADDNOTES must be passed without other content flags than IDF_NOTE
    bool bAddNotes = (nInsFlag & (IDF_CONTENTS | IDF_ADDNOTES)) == (IDF_NOTE | IDF_ADDNOTES);

    BOOL bAtEnd = FALSE;
    for (SCSIZE i = 0; i < nColCount && !bAtEnd; i++)
    {
        SCsROW nDestRow = rColumn.pItems[i].nRow + nDy;
        if ( nDestRow > (SCsROW) nRow2 )
            bAtEnd = TRUE;
        else if ( nDestRow >= (SCsROW) nRow1 )
        {
            //  rows at the beginning may be skipped if filtered rows are left out,
            //  nDestRow may be negative then

            ScAddress aDestPos( nCol, (SCROW)nDestRow, nTab );

            /*  #i102056# Paste from clipboard needs to paste the cell notes in
                a second pass. This must not overwrite the existing cells
                already copied to the destination position in the first pass.
                To indicate this special case, the modifier IDF_ADDNOTES is
                passed together with IDF_NOTE in nInsFlag. Of course, there is
                still the need to create a new cell, if there is no cell at the
                destination position at all. */
            ScBaseCell* pAddNoteCell = bAddNotes ? GetCell( aDestPos.Row() ) : 0;
            if (pAddNoteCell)
            {
                // do nothing if source cell does not contain a note
                const ScBaseCell* pSourceCell = rColumn.pItems[i].pCell;
                const ScPostIt* pSourceNote = pSourceCell ? pSourceCell->GetNote() : 0;
                if (pSourceNote)
                {
                    DBG_ASSERT( !pAddNoteCell->HasNote(), "ScColumn::CopyFromClip - unexpected note at destination cell" );
                    bool bCloneCaption = (nInsFlag & IDF_NOCAPTIONS) == 0;
                    // #i52342# if caption is cloned, the note must be constructed with the destination document
                    ScAddress aSourcePos( rColumn.nCol, rColumn.pItems[i].nRow, rColumn.nTab );
                    ScPostIt* pNewNote = pSourceNote->Clone( aSourcePos, *pDocument, aDestPos, bCloneCaption );
                    pAddNoteCell->TakeNote( pNewNote );
                }
            }
            else
            {
                ScBaseCell* pNewCell = bAsLink ?
                    rColumn.CreateRefCell( pDocument, aDestPos, i, nInsFlag ) :
                    rColumn.CloneCell( i, nInsFlag, *pDocument, aDestPos );
                if (pNewCell)
                    Insert( aDestPos.Row(), pNewCell );
            }
        }
    }
}


namespace {

/** Helper for ScColumn::CloneCell - decides whether to clone a value cell depending on clone flags and number format. */
bool lclCanCloneValue( ScDocument& rDoc, const ScColumn& rCol, SCROW nRow, bool bCloneValue, bool bCloneDateTime )
{
    // values and dates, or nothing to be cloned -> not needed to check number format
    if( bCloneValue == bCloneDateTime )
        return bCloneValue;

    // check number format of value cell
    ULONG nNumIndex = (ULONG)((SfxUInt32Item*)rCol.GetAttr( nRow, ATTR_VALUE_FORMAT ))->GetValue();
    short nTyp = rDoc.GetFormatTable()->GetType( nNumIndex );
    bool bIsDateTime = (nTyp == NUMBERFORMAT_DATE) || (nTyp == NUMBERFORMAT_TIME) || (nTyp == NUMBERFORMAT_DATETIME);
    return bIsDateTime ? bCloneDateTime : bCloneValue;
}

} // namespace


ScBaseCell* ScColumn::CloneCell(SCSIZE nIndex, USHORT nFlags, ScDocument& rDestDoc, const ScAddress& rDestPos)
{
    bool bCloneValue    = (nFlags & IDF_VALUE) != 0;
    bool bCloneDateTime = (nFlags & IDF_DATETIME) != 0;
    bool bCloneString   = (nFlags & IDF_STRING) != 0;
    bool bCloneSpecialBoolean  = (nFlags & IDF_SPECIAL_BOOLEAN) != 0;
    bool bCloneFormula  = (nFlags & IDF_FORMULA) != 0;
    bool bCloneNote     = (nFlags & IDF_NOTE) != 0;
    bool bForceFormula  = false;

    ScBaseCell* pNew = 0;
    ScBaseCell& rSource = *pItems[nIndex].pCell;
    switch (rSource.GetCellType())
    {
        case CELLTYPE_NOTE:
            // note will be cloned below
        break;

        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            // note will be cloned below
            if (bCloneString)
                pNew = rSource.CloneWithoutNote( rDestDoc, rDestPos );
        break;

        case CELLTYPE_VALUE:
            // note will be cloned below
            if (lclCanCloneValue( *pDocument, *this, pItems[nIndex].nRow, bCloneValue, bCloneDateTime ))
                pNew = rSource.CloneWithoutNote( rDestDoc, rDestPos );
        break;

        case CELLTYPE_FORMULA:
            if ( bCloneSpecialBoolean )
            {
                ScFormulaCell& rForm = (ScFormulaCell&)rSource;
                rtl::OUStringBuffer aBuf;
                // #TODO #FIXME do we have a localisation issue here?
                rForm.GetFormula( aBuf );
                rtl::OUString aVal( aBuf.makeStringAndClear() );
                if ( aVal.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "=TRUE()" ) )
                        || aVal.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "=FALSE()" ) ) )
                    bForceFormula = true;
            }
            if (bForceFormula || bCloneFormula)
            {
                // note will be cloned below
                pNew = rSource.CloneWithoutNote( rDestDoc, rDestPos );
            }
            else if ( (bCloneValue || bCloneDateTime || bCloneString) && !rDestDoc.IsUndo() )
            {
                //  ins Undo-Dokument immer nur die Original-Zelle kopieren,
                //  aus Formeln keine Value/String-Zellen erzeugen
                ScFormulaCell& rForm = (ScFormulaCell&)rSource;
                USHORT nErr = rForm.GetErrCode();
                if ( nErr )
                {
                    // error codes are cloned with values
                    if (bCloneValue)
                    {
                        ScFormulaCell* pErrCell = new ScFormulaCell( &rDestDoc, rDestPos );
                        pErrCell->SetErrCode( nErr );
                        pNew = pErrCell;
                    }
                }
                else if (rForm.IsValue())
                {
                    if (lclCanCloneValue( *pDocument, *this, pItems[nIndex].nRow, bCloneValue, bCloneDateTime ))
                    {
                        double nVal = rForm.GetValue();
                        pNew = new ScValueCell(nVal);
                    }
                }
                else if (bCloneString)
                {
                    String aString;
                    rForm.GetString( aString );
                    // do not clone empty string
                    if (aString.Len() > 0)
                    {
                        if ( rForm.IsMultilineResult() )
                        {
                            pNew = new ScEditCell( aString, &rDestDoc );
                        }
                        else
                        {
                            pNew = new ScStringCell( aString );
                        }
                    }
                }
            }
        break;

        default: DBG_ERRORFILE( "ScColumn::CloneCell - unknown cell type" );
    }

    // clone the cell note
    if (bCloneNote)
    {
        if (ScPostIt* pNote = rSource.GetNote())
        {
            bool bCloneCaption = (nFlags & IDF_NOCAPTIONS) == 0;
            // #i52342# if caption is cloned, the note must be constructed with the destination document
            ScAddress aOwnPos( nCol, pItems[nIndex].nRow, nTab );
            ScPostIt* pNewNote = pNote->Clone( aOwnPos, rDestDoc, rDestPos, bCloneCaption );
            if (!pNew)
                pNew = new ScNoteCell( pNewNote );
            else
                pNew->TakeNote( pNewNote );
        }
    }

    return pNew;
}


void ScColumn::MixMarked( const ScMarkData& rMark, USHORT nFunction,
                            bool bSkipEmpty, ScColumn& rSrcCol )
{
    SCROW nRow1, nRow2;

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
        const formula::FormulaToken* pToken = pCode->First();
        while (pToken)
        {
            rArr.AddToken( *pToken );
            pToken = pCode->Next();
        }
    }

    rArr.AddOpCode(ocClose);
}


void ScColumn::MixData( SCROW nRow1, SCROW nRow2,
                            USHORT nFunction, bool bSkipEmpty,
                            ScColumn& rSrcCol )
{
    SCSIZE nSrcCount = rSrcCol.nCount;

    SCSIZE nIndex;
    Search( nRow1, nIndex );

//  SCSIZE nSrcIndex = 0;
    SCSIZE nSrcIndex;
    rSrcCol.Search( nRow1, nSrcIndex );         //! Testen, ob Daten ganz vorne

    SCROW nNextThis = MAXROW+1;
    if ( nIndex < nCount )
        nNextThis = pItems[nIndex].nRow;
    SCROW nNextSrc = MAXROW+1;
    if ( nSrcIndex < nSrcCount )
        nNextSrc = rSrcCol.pItems[nSrcIndex].nRow;

    while ( nNextThis <= nRow2 || nNextSrc <= nRow2 )
    {
        SCROW nRow = Min( nNextThis, nNextSrc );

        ScBaseCell* pSrc = NULL;
        ScBaseCell* pDest = NULL;
        ScBaseCell* pNew = NULL;
        BOOL bDelete = FALSE;

        if ( nSrcIndex < nSrcCount && nNextSrc == nRow )
            pSrc = rSrcCol.pItems[nSrcIndex].pCell;

        if ( nIndex < nCount && nNextThis == nRow )
            pDest = pItems[nIndex].pCell;

        DBG_ASSERT( pSrc || pDest, "Nanu ?" );

        CellType eSrcType  = pSrc  ? pSrc->GetCellType()  : CELLTYPE_NONE;
        CellType eDestType = pDest ? pDest->GetCellType() : CELLTYPE_NONE;

        BOOL bSrcEmpty = ( eSrcType == CELLTYPE_NONE || eSrcType == CELLTYPE_NOTE );
        BOOL bDestEmpty = ( eDestType == CELLTYPE_NONE || eDestType == CELLTYPE_NOTE );

        if ( bSkipEmpty && bDestEmpty )     // Originalzelle wiederherstellen
        {
            if ( pSrc )                     // war da eine Zelle?
            {
                pNew = pSrc->CloneWithoutNote( *pDocument );
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
                    pNew = pSrc->CloneWithoutNote( *pDocument );
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


ScAttrIterator* ScColumn::CreateAttrIterator( SCROW nStartRow, SCROW nEndRow ) const
{
    return new ScAttrIterator( pAttrArray, nStartRow, nEndRow );
}


void ScColumn::StartAllListeners()
{
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                SCROW nRow = pItems[i].nRow;
                ((ScFormulaCell*)pCell)->StartListeningTo( pDocument );
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener eingefuegt?
            }
        }
}


void ScColumn::StartNeededListeners()
{
    if (pItems)
    {
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
                if (pFCell->NeedsListening())
                {
                    SCROW nRow = pItems[i].nRow;
                    pFCell->StartListeningTo( pDocument );
                    if ( nRow != pItems[i].nRow )
                        Search( nRow, i );      // Listener eingefuegt?
                }
            }
        }
    }
}


void ScColumn::BroadcastInArea( SCROW nRow1, SCROW nRow2 )
{
    if ( pItems )
    {
        SCROW nRow;
        SCSIZE nIndex;
        Search( nRow1, nIndex );
        while ( nIndex < nCount && (nRow = pItems[nIndex].nRow) <= nRow2 )
        {
            ScBaseCell* pCell = pItems[nIndex].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->SetDirty();
            else
                pDocument->Broadcast( ScHint( SC_HINT_DATACHANGED,
                    ScAddress( nCol, nRow, nTab ), pCell ) );
            nIndex++;
        }
    }
}


void ScColumn::StartListeningInArea( SCROW nRow1, SCROW nRow2 )
{
    if ( pItems )
    {
        SCROW nRow;
        SCSIZE nIndex;
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
bool ScColumn::SetString( SCROW nRow, SCTAB nTabP, const String& rString,
                          formula::FormulaGrammar::AddressConvention eConv,
                          ScSetStringParam* pParam )
{
    bool bNumFmtSet = false;
    if (!ValidRow(nRow))
        return false;

    ScBaseCell* pNewCell = NULL;
    BOOL bIsLoading = FALSE;
    if (rString.Len() > 0)
    {
        ScSetStringParam aParam;
        if (pParam)
            aParam = *pParam;

        sal_uInt32 nIndex, nOldIndex = 0;
        sal_Unicode cFirstChar;
        if (!aParam.mpNumFormatter)
            aParam.mpNumFormatter = pDocument->GetFormatTable();
        SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
        if ( pDocSh )
            bIsLoading = pDocSh->IsLoading();
        // IsLoading bei ConvertFrom Import
        if ( !bIsLoading )
        {
            nIndex = nOldIndex = GetNumberFormat( nRow );
            if ( rString.Len() > 1
                    && aParam.mpNumFormatter->GetType(nIndex) != NUMBERFORMAT_TEXT )
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
                    ScAddress( nCol, nRow, nTabP ), rString,
                    formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_DEFAULT,
                        eConv), MM_NONE );
        }
        else if ( cFirstChar == '\'')                       // 'Text
            pNewCell = new ScStringCell( rString.Copy(1) );
        else
        {
            double nVal;
            BOOL bIsText = FALSE;
            if ( bIsLoading )
            {
                if ( pItems && nCount )
                {
                    String aStr;
                    SCSIZE i = nCount;
                    SCSIZE nStop = (i >= 3 ? i - 3 : 0);
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
                    nIndex = nOldIndex = aParam.mpNumFormatter->GetStandardIndex();
            }

            do
            {
                if (bIsText)
                    break;

                if (aParam.mbDetectNumberFormat)
                {
                    if (!aParam.mpNumFormatter->IsNumberFormat(rString, nIndex, nVal))
                        break;

                    if ( aParam.mpNumFormatter )
                    {
                        // convert back to the original language if a built-in format was detected
                        const SvNumberformat* pOldFormat = aParam.mpNumFormatter->GetEntry( nOldIndex );
                        if ( pOldFormat )
                            nIndex = aParam.mpNumFormatter->GetFormatForLanguageIfBuiltIn( nIndex, pOldFormat->GetLanguage() );
                    }

                    pNewCell = new ScValueCell( nVal );
                    if ( nIndex != nOldIndex)
                    {
                        // #i22345# New behavior: Apply the detected number format only if
                        // the old one was the default number, date, time or boolean format.
                        // Exception: If the new format is boolean, always apply it.

                        BOOL bOverwrite = FALSE;
                        const SvNumberformat* pOldFormat = aParam.mpNumFormatter->GetEntry( nOldIndex );
                        if ( pOldFormat )
                        {
                            short nOldType = pOldFormat->GetType() & ~NUMBERFORMAT_DEFINED;
                            if ( nOldType == NUMBERFORMAT_NUMBER || nOldType == NUMBERFORMAT_DATE ||
                                 nOldType == NUMBERFORMAT_TIME || nOldType == NUMBERFORMAT_LOGICAL )
                            {
                                if ( nOldIndex == aParam.mpNumFormatter->GetStandardFormat(
                                                    nOldType, pOldFormat->GetLanguage() ) )
                                {
                                    bOverwrite = TRUE;      // default of these types can be overwritten
                                }
                            }
                        }
                        if ( !bOverwrite && aParam.mpNumFormatter->GetType( nIndex ) == NUMBERFORMAT_LOGICAL )
                        {
                            bOverwrite = TRUE;              // overwrite anything if boolean was detected
                        }

                        if ( bOverwrite )
                        {
                            ApplyAttr( nRow, SfxUInt32Item( ATTR_VALUE_FORMAT,
                                (UINT32) nIndex) );
                            bNumFmtSet = true;
                        }
                    }
                }
                else
                {
                    // Only check if the string is a regular number.
                    const LocaleDataWrapper* pLocale = aParam.mpNumFormatter->GetLocaleData();
                    if (!pLocale)
                        break;

                    LocaleDataItem aLocaleItem = pLocale->getLocaleItem();
                    const OUString& rDecSep = aLocaleItem.decimalSeparator;
                    const OUString& rGroupSep = aLocaleItem.thousandSeparator;
                    if (rDecSep.getLength() != 1 || rGroupSep.getLength() != 1)
                        break;

                    sal_Unicode dsep = rDecSep.getStr()[0];
                    sal_Unicode gsep = rGroupSep.getStr()[0];

                    if (!ScStringUtil::parseSimpleNumber(rString, dsep, gsep, nVal))
                        break;

                    pNewCell = new ScValueCell(nVal);
                }
            }
            while (false);

            if (!pNewCell)
            {
                if (aParam.mbSetTextCellFormat && aParam.mpNumFormatter->IsNumberFormat(rString, nIndex, nVal))
                {
                    // Set the cell format type to Text.
                    sal_uInt32 nFormat = aParam.mpNumFormatter->GetStandardFormat(NUMBERFORMAT_TEXT);
                    ScPatternAttr aNewAttrs(pDocument->GetPool());
                    SfxItemSet& rSet = aNewAttrs.GetItemSet();
                    rSet.Put( SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat) );
                    ApplyPattern(nRow, aNewAttrs);
                }

                pNewCell = new ScStringCell(rString);
            }
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
        SCSIZE i;
        if (Search(nRow, i))
        {
            ScBaseCell* pOldCell = pItems[i].pCell;
            ScPostIt* pNote = pOldCell->ReleaseNote();
            SvtBroadcaster* pBC = pOldCell->ReleaseBroadcaster();
            if (pNewCell || pNote || pBC)
            {
                if (pNewCell)
                    pNewCell->TakeNote( pNote );
                else
                    pNewCell = new ScNoteCell( pNote );
                if (pBC)
                {
                    pNewCell->TakeBroadcaster(pBC);
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
                    pDocument->Broadcast( ScHint( SC_HINT_DATACHANGED,
                        ScAddress( nCol, nRow, nTabP ), pNewCell ) );
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

    return bNumFmtSet;
}


void ScColumn::GetFilterEntries(SCROW nStartRow, SCROW nEndRow, TypedScStrCollection& rStrings, bool& rHasDates)
{
    bool bHasDates = false;
    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
    String aString;
    SCROW nRow = 0;
    SCSIZE nIndex;

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

            if (pFormatter)
            {
                short nType = pFormatter->GetType(nFormat);
                if ((nType & NUMBERFORMAT_DATE) && !(nType & NUMBERFORMAT_TIME))
                {
                    // special case for date values.  Disregard the time
                    // element if the number format is of date type.
                    nValue = ::rtl::math::approxFloor(nValue);
                    bHasDates = true;
                }
            }

            pData = new TypedStrData( aString, nValue, SC_STRTYPE_VALUE );
        }

        if ( !rStrings.Insert( pData ) )
            delete pData;                               // doppelt

        ++nIndex;
    }

    rHasDates = bHasDates;
}

//
//  GetDataEntries - Strings aus zusammenhaengendem Bereich um nRow
//

//  DATENT_MAX      - max. Anzahl Eintrage in Liste fuer Auto-Eingabe
//  DATENT_SEARCH   - max. Anzahl Zellen, die durchsucht werden - neu: nur Strings zaehlen
#define DATENT_MAX      200
#define DATENT_SEARCH   2000


bool ScColumn::GetDataEntries(SCROW nStartRow, TypedScStrCollection& rStrings, bool bLimit)
{
    BOOL bFound = FALSE;
    SCSIZE nThisIndex;
    BOOL bThisUsed = Search( nStartRow, nThisIndex );
    String aString;
    USHORT nCells = 0;

    //  Die Beschraenkung auf angrenzende Zellen (ohne Luecken) ist nicht mehr gewollt
    //  (Featurekommission zur 5.1), stattdessen abwechselnd nach oben und unten suchen,
    //  damit naheliegende Zellen wenigstens zuerst gefunden werden.
    //! Abstaende der Zeilennummern vergleichen? (Performance??)

    SCSIZE nUpIndex = nThisIndex;       // zeigt hinter die Zelle
    SCSIZE nDownIndex = nThisIndex;     // zeigt auf die Zelle
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


void ScColumn::RemoveProtected( SCROW nStartRow, SCROW nEndRow )
{
    ScAttrIterator aAttrIter( pAttrArray, nStartRow, nEndRow );
    SCROW nTop = -1;
    SCROW nBottom = -1;
    SCSIZE nIndex;
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


void ScColumn::SetError( SCROW nRow, const USHORT nError)
{
    if (VALIDROW(nRow))
    {
        ScFormulaCell* pCell = new ScFormulaCell
            ( pDocument, ScAddress( nCol, nRow, nTab ) );
        pCell->SetErrCode( nError );
        Insert( nRow, pCell );
    }
}


void ScColumn::SetValue( SCROW nRow, const double& rVal)
{
    if (VALIDROW(nRow))
    {
        ScBaseCell* pCell = new ScValueCell(rVal);
        Insert( nRow, pCell );
    }
}


void ScColumn::GetString( SCROW nRow, String& rString ) const
{
    SCSIZE  nIndex;
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


void ScColumn::GetInputString( SCROW nRow, String& rString ) const
{
    SCSIZE  nIndex;
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


double ScColumn::GetValue( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        switch (pCell->GetCellType())
        {
            case CELLTYPE_VALUE:
                return ((ScValueCell*)pCell)->GetValue();
//                break;
            case CELLTYPE_FORMULA:
                {
                    if (((ScFormulaCell*)pCell)->IsValue())
                        return ((ScFormulaCell*)pCell)->GetValue();
                    else
                        return 0.0;
                }
//                break;
            default:
                return 0.0;
//                break;
        }
    }
    return 0.0;
}


void ScColumn::GetFormula( SCROW nRow, String& rFormula ) const
{
    SCSIZE  nIndex;
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


CellType ScColumn::GetCellType( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
        return pItems[nIndex].pCell->GetCellType();
    return CELLTYPE_NONE;
}


USHORT ScColumn::GetErrCode( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
            return ((ScFormulaCell*)pCell)->GetErrCode();
    }
    return 0;
}


bool ScColumn::HasStringData( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
        return (pItems[nIndex].pCell)->HasStringData();
    return FALSE;
}


bool ScColumn::HasValueData( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
        return (pItems[nIndex].pCell)->HasValueData();
    return FALSE;
}

bool ScColumn::HasStringCells( SCROW nStartRow, SCROW nEndRow ) const
{
    //  TRUE, wenn String- oder Editzellen im Bereich

    if ( pItems )
    {
        SCSIZE nIndex;
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


ScPostIt* ScColumn::GetNote( SCROW nRow )
{
    SCSIZE nIndex;
    return Search( nRow, nIndex ) ? pItems[ nIndex ].pCell->GetNote() : 0;
}


void ScColumn::TakeNote( SCROW nRow, ScPostIt* pNote )
{
    SCSIZE nIndex;
    if( Search( nRow, nIndex ) )
        pItems[ nIndex ].pCell->TakeNote( pNote );
    else
        Insert( nRow, new ScNoteCell( pNote ) );
}


ScPostIt* ScColumn::ReleaseNote( SCROW nRow )
{
    ScPostIt* pNote = 0;
    SCSIZE nIndex;
    if( Search( nRow, nIndex ) )
    {
        ScBaseCell* pCell = pItems[ nIndex ].pCell;
        pNote = pCell->ReleaseNote();
        if( (pCell->GetCellType() == CELLTYPE_NOTE) && !pCell->GetBroadcaster() )
            DeleteAtIndex( nIndex );
    }
    return pNote;
}


void ScColumn::DeleteNote( SCROW nRow )
{
    delete ReleaseNote( nRow );
}


sal_Int32 ScColumn::GetMaxStringLen( SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const
{
    sal_Int32 nStringLen = 0;
    if ( pItems )
    {
        String aString;
        rtl::OString aOString;
        bool bIsOctetTextEncoding = rtl_isOctetTextEncoding( eCharSet);
        SvNumberFormatter* pNumFmt = pDocument->GetFormatTable();
        SCSIZE nIndex;
        SCROW nRow;
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
                sal_Int32 nLen;
                if (bIsOctetTextEncoding)
                {
                    rtl::OUString aOUString( aString);
                    if (!aOUString.convertToString( &aOString, eCharSet,
                                RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                                RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR))
                    {
                        // TODO: anything? this is used by the dBase export filter
                        // that throws an error anyway, but in case of another
                        // context we might want to indicate a conversion error
                        // early.
                    }
                    nLen = aOString.getLength();
                }
                else
                    nLen = aString.Len() * sizeof(sal_Unicode);
                if ( nStringLen < nLen)
                    nStringLen = nLen;
            }
            nIndex++;
        }
    }
    return nStringLen;
}


xub_StrLen ScColumn::GetMaxNumberStringLen(
    sal_uInt16& nPrecision, SCROW nRowStart, SCROW nRowEnd ) const
{
    xub_StrLen nStringLen = 0;
    nPrecision = pDocument->GetDocOptions().GetStdPrecision();
    if ( nPrecision == SvNumberFormatter::UNLIMITED_PRECISION )
        // In case of unlimited precision, use 2 instead.
        nPrecision = 2;

    if ( pItems )
    {
        String aString;
        SvNumberFormatter* pNumFmt = pDocument->GetFormatTable();
        SCSIZE nIndex;
        SCROW nRow;
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
                    {
                        const SvNumberformat* pEntry = pNumFmt->GetEntry( nFormat );
                        sal_uInt16 nPrec;
                        if (pEntry)
                        {
                            BOOL bThousand, bNegRed;
                            USHORT nLeading;
                            pEntry->GetFormatSpecialInfo(bThousand, bNegRed, nPrec, nLeading);
                        }
                        else
                            nPrec = pNumFmt->GetFormatPrecision( nFormat );

                        if ( nPrec != SvNumberFormatter::UNLIMITED_PRECISION && nPrec > nPrecision )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
