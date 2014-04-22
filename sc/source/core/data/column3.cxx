/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <boost/scoped_ptr.hpp>

#include <mdds/flat_segment_tree.hpp>

#include <sfx2/objsh.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/broadcast.hxx>

#include "scitems.hxx"
#include "column.hxx"
#include "cell.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "typedstrdata.hxx"
#include "formula/errorcodes.hxx"
#include "formula/token.hxx"
#include "brdcst.hxx"
#include "docoptio.hxx"         // GetStdPrecision for GetMaxNumberStringLen
#include "subtotal.hxx"
#include "markdata.hxx"
#include "detfunc.hxx"          // For Notes for DeleteRange
#include "postit.hxx"
#include "stringutil.hxx"
#include "docpool.hxx"
#include "globalnames.hxx"
#include "cellvalue.hxx"
#include "tokenarray.hxx"
#include "stlalgorithm.hxx"
#include "clipcontext.hxx"

#include <com/sun/star/i18n/LocaleDataItem.hpp>

#include <cstdio>

using ::com::sun::star::i18n::LocaleDataItem;

// Err527 Workaroand
extern const ScFormulaCell* pLastFormulaTreeTop; // in cellform.cxx
using namespace formula;
// STATIC DATA -----------------------------------------------------------

namespace {

void broadcastCells(ScDocument& rDoc, SCCOL nCol, SCROW nTab, const std::vector<SCROW>& rRows)
{
    // Broadcast the changes.
    ScHint aHint(SC_HINT_DATACHANGED, ScAddress(nCol, 0, nTab));
    std::vector<SCROW>::const_iterator itRow = rRows.begin(), itRowEnd = rRows.end();
    for (; itRow != itRowEnd; ++itRow)
    {
        aHint.GetAddress().SetRow(*itRow);
        rDoc.Broadcast(aHint);
    }
}

}

void ScColumn::Insert( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, ScBaseCell* pNewCell )
{
    SetCell(rBlockPos, nRow, pNewCell);
    PostSetCell(nRow, pNewCell);
}

void ScColumn::Insert( SCROW nRow, ScBaseCell* pNewCell )
{
    SetCell(nRow, pNewCell);
    PostSetCell(nRow, pNewCell);
}


void ScColumn::Insert( SCROW nRow, sal_uInt32 nNumberFormat, ScBaseCell* pCell )
{
    Insert(nRow, pCell);
    SetNumberFormat(nRow, nNumberFormat);
}

void ScColumn::Append( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, ScBaseCell* pCell )
{
    maItems.push_back(ColEntry());
    maItems.back().pCell = pCell;
    maItems.back().nRow  = nRow;

    rBlockPos.miCellTextAttrPos =
        maCellTextAttrs.set(rBlockPos.miCellTextAttrPos, nRow, sc::CellTextAttr());

    CellStorageModified();
}

void ScColumn::Append( SCROW nRow, ScBaseCell* pCell )
{
    maItems.push_back(ColEntry());
    maItems.back().pCell = pCell;
    maItems.back().nRow  = nRow;

    maCellTextAttrs.set<sc::CellTextAttr>(nRow, sc::CellTextAttr());
    CellStorageModified();
}

void ScColumn::Delete( SCROW nRow )
{
    SCSIZE  nIndex;
    if (!Search(nRow, nIndex))
        return;

    ScBaseCell* pCell = maItems[nIndex].pCell;
    maItems.erase(maItems.begin() + nIndex);
    maCellTextAttrs.set_empty(nRow, nRow);
    // Should we free memory here (delta)? It'll be slower!
    if (pCell->GetCellType() == CELLTYPE_FORMULA)
        static_cast<ScFormulaCell*>(pCell)->EndListeningTo(pDocument);
    pCell->Delete();

    pDocument->Broadcast(
        ScHint(SC_HINT_DATACHANGED, ScAddress(nCol, nRow, nTab)));

    CellStorageModified();
}


void ScColumn::DeleteAtIndex( SCSIZE nIndex )
{
    ScBaseCell* pCell = maItems[nIndex].pCell;
    SCROW nRow = maItems[nIndex].nRow;
    maItems.erase(maItems.begin() + nIndex);
    if (pCell->GetCellType() == CELLTYPE_FORMULA)
        static_cast<ScFormulaCell*>(pCell)->EndListeningTo(pDocument);
    pCell->Delete();

    pDocument->Broadcast(
        ScHint(SC_HINT_DATACHANGED, ScAddress(nCol, nRow, nTab)));

    maCellTextAttrs.set_empty(nRow, nRow);
    CellStorageModified();
}


void ScColumn::FreeAll()
{
    for (SCSIZE i = 0; i < maItems.size(); i++)
        maItems[i].pCell->Delete();
    maItems.clear();

    // Text width should keep a logical empty range of 0-MAXROW when the cell array is empty.
    maCellTextAttrs.clear();
    maCellTextAttrs.resize(MAXROWCOUNT);
    CellStorageModified();
}


void ScColumn::DeleteRow( SCROW nStartRow, SCSIZE nSize )
{
    SCROW nEndRow = nStartRow + nSize - 1;

    pAttrArray->DeleteRow( nStartRow, nSize );

    maBroadcasters.erase(nStartRow, nEndRow);
    maBroadcasters.resize(MAXROWCOUNT);

    if ( maItems.empty() )
        return ;

    SCSIZE nFirstIndex;
    Search( nStartRow, nFirstIndex );
    if ( nFirstIndex >= maItems.size() )
        return ;

    sal_Bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false ); // Avoid calculating it multiple times

    bool bFound = false;
    SCSIZE nStartIndex = 0;
    SCSIZE nEndIndex = 0;
    SCSIZE i;


    for ( i = nFirstIndex; i < maItems.size() && maItems[i].nRow <= nEndRow; i++ )
    {
        if (!bFound)
        {
            nStartIndex = i;
            bFound = true;
        }
        nEndIndex = i;
    }

    if (bFound)
    {
        std::vector<SCROW> aDeletedRows;
        DeleteRange(nStartIndex, nEndIndex, IDF_CONTENTS, aDeletedRows);
        broadcastCells(*pDocument, nCol, nTab, aDeletedRows);

        Search( nStartRow, i );
        if ( i >= maItems.size() )
        {
            pDocument->SetAutoCalc( bOldAutoCalc );
            return ;
        }
    }
    else
        i = nFirstIndex;

    // There are cells below the deletion point.  Shift their row positions.

    // Shift the text width array too (before the broadcast).
    maCellTextAttrs.erase(nStartRow, nEndRow);
    maCellTextAttrs.resize(MAXROWCOUNT);

    ScAddress aAdr( nCol, 0, nTab );
    ScHint aHint(SC_HINT_DATACHANGED, aAdr); // only areas (ScBaseCell* == NULL)
    ScAddress& rAddress = aHint.GetAddress();
    // for sparse occupation use single broadcasts, not ranges
    bool bSingleBroadcasts = (((maItems.back().nRow - maItems[i].nRow) /
                (maItems.size() - i)) > 1);
    if ( bSingleBroadcasts )
    {
        SCROW nLastBroadcast = MAXROW+1;
        for ( ; i < maItems.size(); i++ )
        {
            SCROW nOldRow = maItems[i].nRow;
            // Broadcast change in source
            rAddress.SetRow( nOldRow );
            pDocument->AreaBroadcast( aHint );
            SCROW nNewRow = (maItems[i].nRow -= nSize);
            // Broadcast change in target
            if ( nLastBroadcast != nNewRow )
            {   // Do not broadcast successive ones
                rAddress.SetRow( nNewRow );
                pDocument->AreaBroadcast( aHint );
            }
            nLastBroadcast = nOldRow;
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
        }
    }
    else
    {
        rAddress.SetRow( maItems[i].nRow );
        ScRange aRange( rAddress );
        aRange.aEnd.SetRow( maItems.back().nRow );
        for ( ; i < maItems.size(); i++ )
        {
            SCROW nNewRow = (maItems[i].nRow -= nSize);
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
        }
        pDocument->AreaBroadcastInRange( aRange, aHint );
    }

    CellStorageModified();
    pDocument->SetAutoCalc( bOldAutoCalc );
}

void ScColumn::UpdateScriptType( sc::CellTextAttr& rAttr, SCROW nRow )
{
    if (rAttr.mnScriptType != SC_SCRIPTTYPE_UNKNOWN)
        // Already updated. Nothing to do.
        return;

    // Script type not yet determined. Determine the real script
    // type, and store it.
    const ScPatternAttr* pPattern = GetPattern(nRow);
    if (!pPattern)
        return;

    ScRefCellValue aCell;
    ScAddress aPos(nCol, nRow, nTab);
    aCell.assign(*pDocument, aPos);

    const SfxItemSet* pCondSet = NULL;
    ScConditionalFormatList* pCFList = pDocument->GetCondFormList(nTab);
    if (pCFList)
    {
        const ScCondFormatItem& rItem =
            static_cast<const ScCondFormatItem&>(pPattern->GetItem(ATTR_CONDITIONAL));
        const std::vector<sal_uInt32>& rData = rItem.GetCondFormatData();
        pCondSet = pDocument->GetCondResult(aCell, aPos, *pCFList, rData);
    }

    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();

    OUString aStr;
    Color* pColor;
    sal_uLong nFormat = pPattern->GetNumberFormat(pFormatter, pCondSet);
    ScCellFormat::GetString(aCell, nFormat, aStr, &pColor, *pFormatter, pDocument);

    // Store the real script type to the array.
    rAttr.mnScriptType = pDocument->GetStringScriptType(aStr);
}

namespace {

bool isDate(const ScDocument& rDoc, const ScColumn& rCol, SCROW nRow)
{
    sal_uLong nIndex = (sal_uLong)((SfxUInt32Item*)rCol.GetAttr(nRow, ATTR_VALUE_FORMAT))->GetValue();
    short nType = rDoc.GetFormatTable()->GetType(nIndex);
    return (nType == NUMBERFORMAT_DATE) || (nType == NUMBERFORMAT_TIME) || (nType == NUMBERFORMAT_DATETIME);
}

bool checkDeleteCellByFlag(
    CellType eCellType, sal_uInt16 nDelFlag, const ScDocument& rDoc, const ScColumn& rCol, const ColEntry& rEntry)
{
    bool bDelete = false;

    switch (eCellType)
    {
        case CELLTYPE_VALUE:
        {
            sal_uInt16 nValFlags = nDelFlag & (IDF_DATETIME|IDF_VALUE);
            // delete values and dates?
            bDelete = nValFlags == (IDF_DATETIME|IDF_VALUE);
            // if not, decide according to cell number format
            if (!bDelete && (nValFlags != 0))
            {
                bool bIsDate = isDate(rDoc, rCol, rEntry.nRow);
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
        default:; // added to avoid warnings
    }

    return bDelete;
}

}

void ScColumn::DeleteRange(
    SCSIZE nStartIndex, SCSIZE nEndIndex, sal_uInt16 nDelFlag, std::vector<SCROW>& rDeletedRows )
{
    /*  If caller specifies to not remove the note caption objects, all cells
        have to forget the pointers to them. This is used e.g. while undoing a
        "paste cells" operation, which removes the caption objects later in
        drawing undo. */

    // cache all formula cells, they will be deleted at end of this function
    std::vector<ScFormulaCell*> aDelCells;
    aDelCells.reserve( nEndIndex - nStartIndex + 1 );

    typedef mdds::flat_segment_tree<SCSIZE, bool> RemovedSegments_t;
    RemovedSegments_t aRemovedSegments(nStartIndex, maItems.size(), false);
    SCSIZE nFirst = nStartIndex;

    for ( SCSIZE nIdx = nStartIndex; nIdx <= nEndIndex; ++nIdx )
    {
        if (((nDelFlag & IDF_CONTENTS) == IDF_CONTENTS))
        {
            // all content is to be deleted.

            ScBaseCell* pOldCell = maItems[ nIdx ].pCell;
            rDeletedRows.push_back(maItems[nIdx].nRow);

            if (pOldCell->GetCellType() == CELLTYPE_FORMULA)
            {
                // cache formula cell, will be deleted below
                aDelCells.push_back( static_cast< ScFormulaCell* >( pOldCell ) );
            }
            else
                pOldCell->Delete();

            continue;
        }

        // delete some contents of the cells, or cells with broadcaster
        bool bDelete = false;
        ScBaseCell* pOldCell = maItems[nIdx].pCell;
        CellType eCellType = pOldCell->GetCellType();
        if ((nDelFlag & IDF_CONTENTS) == IDF_CONTENTS)
            // All cell types to be deleted.
            bDelete = true;
        else
        {
            // Decide whether to delete the cell object according to passed
            // flags.
            bDelete = checkDeleteCellByFlag(eCellType, nDelFlag, *pDocument, *this, maItems[nIdx]);
        }

        if (bDelete)
        {
            // remove cell entry in cell item list
            if (eCellType == CELLTYPE_FORMULA)
            {
                // Cache formula cells (will be deleted later), delete cell of other type.
                aDelCells.push_back(static_cast<ScFormulaCell*>(pOldCell));
            }
            else
                pOldCell->Delete();

            rDeletedRows.push_back(maItems[nIdx].nRow);
        }

        if (!bDelete)
        {
            // We just came to a non-deleted cell after a segment of
            // deleted ones. So we need to remember the segment
            // before moving on.
            if (nFirst < nIdx)
                aRemovedSegments.insert_back(nFirst, nIdx, true);
            nFirst = nIdx + 1;
        }
    }
    // there is a segment of deleted cells at the end
    if (nFirst <= nEndIndex)
        aRemovedSegments.insert_back(nFirst, nEndIndex + 1, true);

    {
        // Remove segments from the column array, containing pDummyCell and
        // formula cell pointers to be deleted.

        RemovedSegments_t::const_reverse_iterator it = aRemovedSegments.rbegin();
        RemovedSegments_t::const_reverse_iterator itEnd = aRemovedSegments.rend();

        std::vector<ColEntry>::iterator itErase, itEraseEnd;
        SCSIZE nEndSegment = it->first; // should equal maItems.size(). Non-inclusive.
        // Skip the first node.
        for (++it; it != itEnd; ++it)
        {
            if (!it->second)
            {
                // Don't remove this segment.
                nEndSegment = it->first;
                continue;
            }

            // Remove this segment.
            SCSIZE nStartSegment = it->first;
            SCROW nStartRow = maItems[nStartSegment].nRow;
            SCROW nEndRow = maItems[nEndSegment-1].nRow;

            itErase = maItems.begin();
            std::advance(itErase, nStartSegment);
            itEraseEnd = maItems.begin();
            std::advance(itEraseEnd, nEndSegment);
            maItems.erase(itErase, itEraseEnd);

            maCellTextAttrs.set_empty(nStartRow, nEndRow);

            nEndSegment = nStartSegment;
        }
    }

    pDocument->EndListeningFormulaCells(aDelCells);
    std::for_each(aDelCells.begin(), aDelCells.end(), ScDeleteObjectByPtr<ScFormulaCell>());
}

void ScColumn::DeleteArea(SCROW nStartRow, SCROW nEndRow, sal_uInt16 nDelFlag)
{
    //  FreeAll must not be called here due to Broadcasters
    //  Delete attribute at the end so that we can distinguish between numbers and dates

    sal_uInt16 nContMask = IDF_CONTENTS;
    // IDF_NOCAPTIONS needs to be passed too, if IDF_NOTE is set
    if( nDelFlag & IDF_NOTE )
        nContMask |= IDF_NOCAPTIONS;
    sal_uInt16 nContFlag = nDelFlag & nContMask;

    std::vector<SCROW> aDeletedRows;

    if ( !maItems.empty() && nContFlag)
    {
        if (nStartRow==0 && nEndRow==MAXROW)
        {
            DeleteRange(0, maItems.size()-1, nContFlag, aDeletedRows);
        }
        else
        {
            sal_Bool bFound=false;
            SCSIZE nStartIndex = 0;
            SCSIZE nEndIndex = 0;
            for (SCSIZE i = 0; i < maItems.size(); i++)
                if ((maItems[i].nRow >= nStartRow) && (maItems[i].nRow <= nEndRow))
                {
                    if (!bFound)
                    {
                        nStartIndex = i;
                        bFound = sal_True;
                    }
                    nEndIndex = i;
                }
            if (bFound)
                DeleteRange(nStartIndex, nEndIndex, nContFlag, aDeletedRows);
        }
    }

    if ( nDelFlag & IDF_EDITATTR )
    {
        OSL_ENSURE( nContFlag == 0, "DeleteArea: Wrong Flags" );
        RemoveEditAttribs( nStartRow, nEndRow );
    }

    // Delete attributes just now
    if ((nDelFlag & IDF_ATTRIB) == IDF_ATTRIB)
        pAttrArray->DeleteArea( nStartRow, nEndRow );
    else if ((nDelFlag & IDF_HARDATTR) == IDF_HARDATTR)
        pAttrArray->DeleteHardAttr( nStartRow, nEndRow );

    // Broadcast on only cells that were deleted; no point broadcasting on
    // cells that were already empty before the deletion.
    broadcastCells(*pDocument, nCol, nTab, aDeletedRows);
}


ScFormulaCell* ScColumn::CreateRefCell( ScDocument* pDestDoc, const ScAddress& rDestPos,
                                            SCSIZE nIndex, sal_uInt16 nFlags ) const
{
    sal_uInt16 nContFlags = nFlags & IDF_CONTENTS;
    if (!nContFlags)
        return NULL;

    // Test whether the Cell should be copied
    // Also do this for IDF_CONTENTS, due to Notes/Broadcasters
    sal_Bool bMatch = false;
    ScBaseCell* pCell = maItems[nIndex].pCell;
    CellType eCellType = pCell->GetCellType();
    switch ( eCellType )
    {
        case CELLTYPE_VALUE:
            {
                sal_uInt16 nValFlags = nFlags & (IDF_DATETIME|IDF_VALUE);

                if ( nValFlags == (IDF_DATETIME|IDF_VALUE) )
                    bMatch = sal_True;
                else if ( nValFlags )
                {
                    sal_uLong nNumIndex = (sal_uLong)((SfxUInt32Item*)GetAttr(
                                    maItems[nIndex].nRow, ATTR_VALUE_FORMAT ))->GetValue();
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


    // Insert Reference
    ScSingleRefData aRef;
    aRef.nCol = nCol;
    aRef.nRow = maItems[nIndex].nRow;
    aRef.nTab = nTab;
    aRef.InitFlags(); // -> Everything absolute
    aRef.SetFlag3D(true);

    // 3D (false) and TabRel (true), if the final Position is at the same Table?
    // The target position is not yet known for TransposeClip!

    aRef.CalcRelFromAbs( rDestPos );

    ScTokenArray aArr;
    aArr.AddSingleReference( aRef );

    return new ScFormulaCell( pDestDoc, rDestPos, &aArr );
}

bool ScColumn::InitBlockPosition( sc::ColumnBlockPosition& rBlockPos )
{
    rBlockPos.miBroadcasterPos = maBroadcasters.begin();
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.begin();
    return true;
}

//  rColumn = source
//  nRow1, nRow2 = target position

void ScColumn::CopyFromClip(
    sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2, long nDy, ScColumn& rColumn )
{
    if ((rCxt.getInsertFlag() & IDF_ATTRIB) != 0)
    {
        if (rCxt.isSkipAttrForEmptyCells())
        {
            //  copy only attributes for non-empty cells
            //  (notes are not counted as non-empty here, to match the content behavior)

            SCSIZE nStartIndex;
            rColumn.Search( nRow1-nDy, nStartIndex );
            while ( nStartIndex < rColumn.maItems.size() && rColumn.maItems[nStartIndex].nRow <= nRow2-nDy )
            {
                SCSIZE nEndIndex = nStartIndex;
                SCROW nStartRow = rColumn.maItems[nStartIndex].nRow;
                SCROW nEndRow = nStartRow;

                //  find consecutive non-empty cells
                while ( nEndRow < nRow2-nDy &&
                        nEndIndex+1 < rColumn.maItems.size() &&
                        rColumn.maItems[nEndIndex+1].nRow == nEndRow+1 )
                {
                    ++nEndIndex;
                    ++nEndRow;
                }

                rColumn.pAttrArray->CopyAreaSafe( nStartRow+nDy, nEndRow+nDy, nDy, *pAttrArray );
                nStartIndex = nEndIndex + 1;
            }
        }
        else
            rColumn.pAttrArray->CopyAreaSafe( nRow1, nRow2, nDy, *pAttrArray );
    }
    if ((rCxt.getInsertFlag() & IDF_CONTENTS) == 0)
        return;

    if (rCxt.isAsLink() && rCxt.getInsertFlag() == IDF_ALL)
    {
        // We also reference empty cells for "ALL"
        // IDF_ALL must always contain more flags when compared to "Insert contents" as
        // contents can be selected one by one!

        ReserveSize(maItems.size() + static_cast<SCSIZE>(nRow2-nRow1+1));

        ScAddress aDestPos( nCol, 0, nTab ); // Adapt Row

        //  Create reference (Source Position)
        ScSingleRefData aRef;
        aRef.nCol = rColumn.nCol;
        //  Adapt nRow
        aRef.nTab = rColumn.nTab;
        aRef.InitFlags(); // -> All absolute
        aRef.SetFlag3D(true);

        for (SCROW nDestRow = nRow1; nDestRow <= nRow2; nDestRow++)
        {
            aRef.nRow = nDestRow - nDy; // Source row
            aDestPos.SetRow( nDestRow );

            aRef.CalcRelFromAbs( aDestPos );
            ScTokenArray aArr;
            aArr.AddSingleReference( aRef );
            Insert( nDestRow, new ScFormulaCell( pDocument, aDestPos, &aArr ) );
        }

        return;
    }

    SCSIZE nColCount = rColumn.maItems.size();

    // ignore IDF_FORMULA - "all contents but no formulas" results in the same number of cells
    if ((rCxt.getInsertFlag() & ( IDF_CONTENTS & ~IDF_FORMULA )) == ( IDF_CONTENTS & ~IDF_FORMULA ) && nRow2-nRow1 >= 64)
    {
        //! Always do the Resize from the outside, where the number of repetitions is known
        //! (then it can be removed here)

        ReserveSize(maItems.size() + nColCount);
    }

    sal_Bool bAtEnd = false;
    for (SCSIZE i = 0; i < nColCount && !bAtEnd; i++)
    {
        SCsROW nDestRow = rColumn.maItems[i].nRow + nDy;
        if ( nDestRow > (SCsROW) nRow2 )
            bAtEnd = sal_True;
        else if ( nDestRow >= (SCsROW) nRow1 )
        {
            // rows at the beginning may be skipped if filtered rows are left out,
            // nDestRow may be negative then

            ScAddress aDestPos( nCol, (SCROW)nDestRow, nTab );

            ScBaseCell* pNewCell = rCxt.isAsLink() ?
                rColumn.CreateRefCell(pDocument, aDestPos, i, rCxt.getInsertFlag()) :
                rColumn.CloneCell(i, rCxt.getInsertFlag(), *pDocument, aDestPos);
            if (pNewCell)
            {
                sc::ColumnBlockPosition* p = rCxt.getBlockPosition(nTab, nCol);
                if (p)
                    Insert(*p, aDestPos.Row(), pNewCell);
                else
                    Insert(aDestPos.Row(), pNewCell);
            }
        }
    }
}


namespace {

/**
 * Helper for ScColumn::CloneCell
 * Decide whether to clone a value cell depending on clone flags and number format.
 */
bool lclCanCloneValue( ScDocument& rDoc, const ScColumn& rCol, SCROW nRow, bool bCloneValue, bool bCloneDateTime )
{
    // values and dates, or nothing to be cloned -> not needed to check number format
    if( bCloneValue == bCloneDateTime )
        return bCloneValue;

    // check number format of value cell
    sal_uLong nNumIndex = (sal_uLong)((SfxUInt32Item*)rCol.GetAttr( nRow, ATTR_VALUE_FORMAT ))->GetValue();
    short nTyp = rDoc.GetFormatTable()->GetType( nNumIndex );
    bool bIsDateTime = (nTyp == NUMBERFORMAT_DATE) || (nTyp == NUMBERFORMAT_TIME) || (nTyp == NUMBERFORMAT_DATETIME);
    return bIsDateTime ? bCloneDateTime : bCloneValue;
}

} // namespace


ScBaseCell* ScColumn::CloneCell(
    SCSIZE nIndex, sal_uInt16 nFlags, ScDocument& rDestDoc, const ScAddress& rDestPos) const
{
    bool bCloneValue    = (nFlags & IDF_VALUE) != 0;
    bool bCloneDateTime = (nFlags & IDF_DATETIME) != 0;
    bool bCloneString   = (nFlags & IDF_STRING) != 0;
    bool bCloneSpecialBoolean  = (nFlags & IDF_SPECIAL_BOOLEAN) != 0;
    bool bCloneFormula  = (nFlags & IDF_FORMULA) != 0;
    bool bForceFormula  = false;

    ScBaseCell* pNew = 0;
    ScBaseCell& rSource = *maItems[nIndex].pCell;
    switch (rSource.GetCellType())
    {
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            // note will be cloned below
            if (bCloneString)
                pNew = rSource.Clone( rDestDoc, rDestPos );
        break;

        case CELLTYPE_VALUE:
            // note will be cloned below
            if (lclCanCloneValue( *pDocument, *this, maItems[nIndex].nRow, bCloneValue, bCloneDateTime ))
                pNew = rSource.Clone( rDestDoc, rDestPos );
        break;

        case CELLTYPE_FORMULA:
            if ( bCloneSpecialBoolean )
            {
                ScFormulaCell& rForm = (ScFormulaCell&)rSource;
                OUStringBuffer aBuf;
                // FIXME: do we have a localisation issue here?
                rForm.GetFormula( aBuf );
                OUString aVal( aBuf.makeStringAndClear() );
                if ( aVal == "=TRUE()" || aVal == "=FALSE()" )
                    bForceFormula = true;
            }
            if (bForceFormula || bCloneFormula)
            {
                // note will be cloned below
                pNew = rSource.Clone( rDestDoc, rDestPos );
            }
            else if ( (bCloneValue || bCloneDateTime || bCloneString) && !rDestDoc.IsUndo() )
            {
                // Always just copy the original row to the Undo Documen;
                // do not create Value/string cells from formulas
                ScFormulaCell& rForm = (ScFormulaCell&)rSource;
                sal_uInt16 nErr = rForm.GetErrCode();
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
                    if (lclCanCloneValue( *pDocument, *this, maItems[nIndex].nRow, bCloneValue, bCloneDateTime ))
                    {
                        double nVal = rForm.GetValue();
                        pNew = new ScValueCell(nVal);
                    }
                }
                else if (bCloneString)
                {
                    String aString = rForm.GetString();
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

        default: OSL_FAIL( "ScColumn::CloneCell - unknown cell type" );
    }

    return pNew;
}


void ScColumn::MixMarked(
    sc::MixDocContext& rCxt, const ScMarkData& rMark, sal_uInt16 nFunction,
    bool bSkipEmpty, const ScColumn& rSrcCol )
{
    SCROW nRow1, nRow2;

    if (rMark.IsMultiMarked())
    {
        ScMarkArrayIter aIter( rMark.GetArray()+nCol );
        while (aIter.Next( nRow1, nRow2 ))
            MixData(rCxt, nRow1, nRow2, nFunction, bSkipEmpty, rSrcCol);
    }
}

namespace {

// Result in rVal1
bool lcl_DoFunction( double& rVal1, double nVal2, sal_uInt16 nFunction )
{
    bool bOk = false;
    switch (nFunction)
    {
        case PASTE_ADD:
            bOk = SubTotal::SafePlus( rVal1, nVal2 );
            break;
        case PASTE_SUB:
            nVal2 = -nVal2;     // FIXME: Can we do this alwyas without error?
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

struct FindRemovedCell : std::unary_function<ColEntry, bool>
{
    bool operator() (const ColEntry& rEntry) const
    {
        return rEntry.pCell == NULL;
    }
};

}

void ScColumn::MixData(
    sc::MixDocContext& rCxt, SCROW nRow1, SCROW nRow2, sal_uInt16 nFunction,
    bool bSkipEmpty, const ScColumn& rSrcCol )
{
    SCSIZE nSrcCount = rSrcCol.maItems.size();

    sc::ColumnBlockPosition* p = rCxt.getBlockPosition(nTab, nCol);

    SCSIZE nIndex;
    Search( nRow1, nIndex );

    SCSIZE nSrcIndex = 0, nDestIndex = 0;
    rSrcCol.Search( nRow1, nSrcIndex ); // See if data is at the beginning

    SCROW nNextThis = MAXROW+1;
    if ( nIndex < maItems.size() )
        nNextThis = maItems[nIndex].nRow;
    SCROW nNextSrc = MAXROW+1;
    if ( nSrcIndex < nSrcCount )
        nNextSrc = rSrcCol.maItems[nSrcIndex].nRow;

    bool bDeferredDelete = false;
    while ( nNextThis <= nRow2 || nNextSrc <= nRow2 )
    {
        SCROW nRow = std::min( nNextThis, nNextSrc );

        ScBaseCell* pSrc = NULL;
        ScBaseCell* pDest = NULL;
        ScBaseCell* pNew = NULL;
        bool bDelete = false;

        if ( nSrcIndex < nSrcCount && nNextSrc == nRow )
            pSrc = rSrcCol.maItems[nSrcIndex].pCell;

        if ( nIndex < maItems.size() && nNextThis == nRow )
        {
            pDest = maItems[nIndex].pCell;
            nDestIndex = nIndex;
        }

        OSL_ENSURE( pSrc || pDest, "What happened?" );

        CellType eSrcType  = pSrc  ? pSrc->GetCellType()  : CELLTYPE_NONE;
        CellType eDestType = pDest ? pDest->GetCellType() : CELLTYPE_NONE;

        bool bSrcEmpty = (eSrcType == CELLTYPE_NONE);
        bool bDestEmpty = (eDestType == CELLTYPE_NONE);

        if ( bSkipEmpty && bDestEmpty ) // Restore original row
        {
            if ( pSrc ) // Did we have a row here?
            {
                pNew = pSrc->Clone( *pDocument );
            }
        }
        else if ( nFunction ) // Really provide calculation function
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

            // Empty row is treated as a value
            sal_Bool bSrcVal  = ( bSrcEmpty || eSrcType == CELLTYPE_VALUE );
            sal_Bool bDestVal  = ( bDestEmpty || eDestType == CELLTYPE_VALUE );

            sal_Bool bSrcText = ( eSrcType == CELLTYPE_STRING ||
                                eSrcType == CELLTYPE_EDIT );
            sal_Bool bDestText = ( eDestType == CELLTYPE_STRING ||
                                eDestType == CELLTYPE_EDIT );

            // Else we only have formulas ...
            if ( bSrcEmpty && bDestEmpty )
            {
                // Both empty -> do nothing
            }
            else if ( bSrcVal && bDestVal )
            {
                // Insterted new value or both have overflown
                sal_Bool bOk = lcl_DoFunction( nVal1, nVal2, nFunction );

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
                // We do no not calculate with texts - alwyas "old" cell, thus pSrc
                if (pSrc)
                    pNew = pSrc->Clone( *pDocument );
                else if (pDest)
                    bDelete = sal_True;
            }
            else
            {
                // Combination of value and at least one formula -> Create formula
                ScTokenArray aArr;

                // First row
                if ( eSrcType == CELLTYPE_FORMULA )
                    lcl_AddCode( aArr, (ScFormulaCell*)pSrc );
                else
                    aArr.AddDouble( nVal1 );

                // Operator
                OpCode eOp = ocAdd;
                switch ( nFunction )
                {
                    case PASTE_ADD: eOp = ocAdd; break;
                    case PASTE_SUB: eOp = ocSub; break;
                    case PASTE_MUL: eOp = ocMul; break;
                    case PASTE_DIV: eOp = ocDiv; break;
                }
                aArr.AddOpCode(eOp); // Function

                // Second row
                if ( eDestType == CELLTYPE_FORMULA )
                    lcl_AddCode( aArr, (ScFormulaCell*)pDest );
                else
                    aArr.AddDouble( nVal2 );

                pNew = new ScFormulaCell( pDocument, ScAddress( nCol, nRow, nTab ), &aArr );
            }
        }


        if ( pNew || bDelete ) // New result?
        {
            if (pDest && !pNew) // Old cell present?
            {
                // Delete the destination cell because the cell was originally
                // empty.  Don't erase its slot in the cell array yet.
                OSL_ASSERT(pDest == maItems[nDestIndex].pCell);
                maItems[nDestIndex].pCell = NULL;

                if (pDest->GetCellType() == CELLTYPE_FORMULA)
                    static_cast<ScFormulaCell*>(pDest)->EndListeningTo(pDocument);
                pDest->Delete();

                bDeferredDelete = true;
            }
            if (pNew)
            {
                if (p)
                    Insert(*p, nRow, pNew);
                else
                    Insert(nRow, pNew); // Insert new one
            }

            Search( nRow, nIndex ); // Everything could have moved
            if (pNew)
                nNextThis = nRow; // nIndex points right at nRow now
            else
                nNextThis = ( nIndex < maItems.size() ) ? maItems[nIndex].nRow : MAXROW+1;
        }

        if ( nNextThis == nRow )
        {
            ++nIndex;
            nNextThis = ( nIndex < maItems.size() ) ? maItems[nIndex].nRow : MAXROW+1;
        }
        if ( nNextSrc == nRow )
        {
            ++nSrcIndex;
            nNextSrc = ( nSrcIndex < nSrcCount ) ?
                            rSrcCol.maItems[nSrcIndex].nRow :
                            MAXROW+1;
        }
    }

    if (bDeferredDelete)
    {
        // Erase all the slots in the cell array where the deleted cells
        // previously occupied.
        std::vector<ColEntry>::iterator it =
            std::remove_if(maItems.begin(), maItems.end(), FindRemovedCell());

        maItems.erase(it, maItems.end());

        // Reset the cell text attriute array to keep it in sync again.
        ResetCellTextAttrs();
    }
}


ScAttrIterator* ScColumn::CreateAttrIterator( SCROW nStartRow, SCROW nEndRow ) const
{
    return new ScAttrIterator( pAttrArray, nStartRow, nEndRow );
}


void ScColumn::StartAllListeners()
{
    if (maItems.empty())
        return;

    for (SCSIZE i = 0; i < maItems.size(); i++)
    {
        ScBaseCell* pCell = maItems[i].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
        {
            SCROW nRow = maItems[i].nRow;
            ((ScFormulaCell*)pCell)->StartListeningTo( pDocument );
            if ( nRow != maItems[i].nRow )
                Search( nRow, i ); // Insert Listener?
        }
    }
}


void ScColumn::StartNeededListeners()
{
    if (maItems.empty())
        return;

    for (SCSIZE i = 0; i < maItems.size(); i++)
    {
        ScBaseCell* pCell = maItems[i].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
        {
            ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
            if (pFCell->NeedsListening())
            {
                SCROW nRow = maItems[i].nRow;
                pFCell->StartListeningTo( pDocument );
                if ( nRow != maItems[i].nRow )
                    Search( nRow, i ); // Insert Listener?
            }
        }
    }
}


void ScColumn::BroadcastInArea( SCROW nRow1, SCROW nRow2 )
{
    if (maItems.empty())
        return;

    SCROW nRow;
    SCSIZE nIndex;
    Search(nRow1, nIndex);

    while ( nIndex < maItems.size() && (nRow = maItems[nIndex].nRow) <= nRow2 )
    {
        ScBaseCell* pCell = maItems[nIndex].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            ((ScFormulaCell*)pCell)->SetDirty();
        else
            pDocument->Broadcast( ScHint(SC_HINT_DATACHANGED,
                ScAddress(nCol, nRow, nTab)));
        nIndex++;
    }
}


void ScColumn::StartListeningInArea( sc::StartListeningContext& rCxt, SCROW nRow1, SCROW nRow2 )
{
    if (maItems.empty())
        return;

    SCROW nRow;
    SCSIZE nIndex;
    Search( nRow1, nIndex );
    while ( nIndex < maItems.size() && (nRow = maItems[nIndex].nRow) <= nRow2 )
    {
        ScBaseCell* pCell = maItems[nIndex].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            ((ScFormulaCell*)pCell)->StartListeningTo(rCxt);
        if ( nRow != maItems[nIndex].nRow )
            Search( nRow, nIndex ); // Inserted via Listening

        ++nIndex;
    }
}

namespace {

void applyTextNumFormat( ScColumn& rCol, ScDocument& rDoc, SCROW nRow, SvNumberFormatter* pFormatter )
{
    sal_uInt32 nFormat = pFormatter->GetStandardFormat(NUMBERFORMAT_TEXT);
    ScPatternAttr aNewAttrs(rDoc.GetPool());
    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
    rCol.ApplyPattern(nRow, aNewAttrs);
}

}

bool ScColumn::ParseString(
    ScCellValue& rCell, SCROW nRow, SCTAB nTabP, const String& rString,
    formula::FormulaGrammar::AddressConvention eConv,
    ScSetStringParam* pParam )
{
    if (!rString.Len())
        return false;

    bool bNumFmtSet = false;

    ScSetStringParam aParam;

    if (pParam)
        aParam = *pParam;

    sal_uInt32 nIndex = 0;
    sal_uInt32 nOldIndex = 0;
    sal_Unicode cFirstChar;
    if (!aParam.mpNumFormatter)
        aParam.mpNumFormatter = pDocument->GetFormatTable();

    nIndex = nOldIndex = GetNumberFormat( nRow );
    if ( rString.Len() > 1
            && aParam.mpNumFormatter->GetType(nIndex) != NUMBERFORMAT_TEXT )
        cFirstChar = rString.GetChar(0);
    else
        cFirstChar = 0; // Text

    if ( cFirstChar == '=' )
    {
        if ( rString.Len() == 1 ) // = Text
            rCell.set(rString);
        else if (aParam.meSetTextNumFormat == ScSetStringParam::Always)
        {
            // Set the cell format type to Text.
            applyTextNumFormat(*this, *pDocument, nRow, aParam.mpNumFormatter);
            rCell.set(rString);
        }
        else // = Formula
            rCell.set(
                new ScFormulaCell(
                    pDocument, ScAddress(nCol, nRow, nTabP), rString,
                    formula::FormulaGrammar::mergeToGrammar(formula::FormulaGrammar::GRAM_DEFAULT, eConv),
                    MM_NONE));
    }
    else if ( cFirstChar == '\'') // 'Text
    {
        bool bNumeric = false;
        if (aParam.mbHandleApostrophe)
        {
            // Cell format is not 'Text', and the first char
            // is an apostrophe. Check if the input is considered a number.
            String aTest = rString.Copy(1);
            double fTest;
            bNumeric = aParam.mpNumFormatter->IsNumberFormat(aTest, nIndex, fTest);
            if (bNumeric)
                // This is a number. Strip out the first char.
                rCell.set(aTest);
        }
        if (!bNumeric)
            // This is normal text. Take it as-is.
            rCell.set(rString);
    }
    else
    {
        double nVal;

        do
        {
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

                rCell.set(nVal);
                if ( nIndex != nOldIndex)
                {
                    // #i22345# New behavior: Apply the detected number format only if
                    // the old one was the default number, date, time or boolean format.
                    // Exception: If the new format is boolean, always apply it.

                    bool bOverwrite = false;
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
                                bOverwrite = true; // default of these types can be overwritten
                            }
                        }
                    }
                    if ( !bOverwrite && aParam.mpNumFormatter->GetType( nIndex ) == NUMBERFORMAT_LOGICAL )
                    {
                        bOverwrite = true; // overwrite anything if boolean was detected
                    }

                    if ( bOverwrite )
                    {
                        ApplyAttr( nRow, SfxUInt32Item( ATTR_VALUE_FORMAT,
                            (sal_uInt32) nIndex) );
                        bNumFmtSet = true;
                    }
                }
            }
            else if (aParam.meSetTextNumFormat != ScSetStringParam::Always)
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

                rCell.set(nVal);
            }
        }
        while (false);

        if (rCell.meType == CELLTYPE_NONE)
        {
            if (aParam.meSetTextNumFormat != ScSetStringParam::Never && aParam.mpNumFormatter->IsNumberFormat(rString, nIndex, nVal))
            {
                // Set the cell format type to Text.
                applyTextNumFormat(*this, *pDocument, nRow, aParam.mpNumFormatter);
            }

            rCell.set(rString);
        }
    }

    return bNumFmtSet;
}

/**
 * Returns true if the cell format was set as well
 */
bool ScColumn::SetString( SCROW nRow, SCTAB nTabP, const String& rString,
                          formula::FormulaGrammar::AddressConvention eConv,
                          ScSetStringParam* pParam )
{
    if (!ValidRow(nRow))
        return false;

    ScCellValue aNewCell;
    bool bNumFmtSet = ParseString(aNewCell, nRow, nTabP, rString, eConv, pParam);
    aNewCell.release(*this, nRow);

    // Do not set Formats and Formulas here anymore!
    // These are queried during output

    return bNumFmtSet;
}

void ScColumn::SetEditText( SCROW nRow, EditTextObject* pEditText )
{
    Insert(nRow, new ScEditCell(pEditText, pDocument));
}

void ScColumn::SetEditText( SCROW nRow, const EditTextObject& rEditText, const SfxItemPool* pEditPool )
{
    Insert(nRow, new ScEditCell(rEditText, pDocument, pEditPool));
}

void ScColumn::SetFormula( SCROW nRow, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram )
{
    ScAddress aPos(nCol, nRow, nTab);
    ScFormulaCell* pCell = new ScFormulaCell(pDocument, aPos, &rArray, eGram);
    sal_uInt32 nCellFormat = GetNumberFormat( nRow );
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
        pCell->SetNeedNumberFormat(true);
    Insert(nRow, pCell);
}

void ScColumn::SetFormula( SCROW nRow, const OUString& rFormula, formula::FormulaGrammar::Grammar eGram )
{
    ScAddress aPos(nCol, nRow, nTab);
    ScFormulaCell* pCell = new ScFormulaCell(pDocument, aPos, rFormula, eGram);

    sal_uInt32 nCellFormat = GetNumberFormat( nRow );
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
        pCell->SetNeedNumberFormat(true);
    Insert(nRow, pCell);
}

void ScColumn::SetFormulaCell( SCROW nRow, ScFormulaCell* pCell )
{
    Insert(nRow, pCell);
}

void ScColumn::GetFilterEntries(SCROW nStartRow, SCROW nEndRow, std::vector<ScTypedStrData>& rStrings, bool& rHasDates)
{
    bool bHasDates = false;
    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
    OUString aString;
    SCSIZE nIndex;

    Search( nStartRow, nIndex );

    for (; nIndex < maItems.size(); ++nIndex)
    {
        SCROW nRow = maItems[nIndex].nRow;
        if (nRow > nEndRow)
            break;

        ScBaseCell* pCell = maItems[nIndex].pCell;
        ScRefCellValue aCell;
        aCell.assign(*maItems[nIndex].pCell);
        sal_uLong nFormat = GetNumberFormat( nRow );

        ScCellFormat::GetInputString(aCell, nFormat, aString, *pFormatter, pDocument);

        if ( pDocument->HasStringData( nCol, nRow, nTab ) )
        {
            rStrings.push_back(ScTypedStrData(aString));
            continue;
        }

        double nValue = 0.0;

        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_VALUE:
                nValue = ((ScValueCell*)pCell)->GetValue();
                break;

            case CELLTYPE_FORMULA:
            {
                ScFormulaCell* pFC = static_cast<ScFormulaCell*>(pCell);
                sal_uInt16 nErr = pFC->GetErrCode();
                if (nErr)
                {
                    // Error cell is evaluated as string (for now).
                    String aErr = ScGlobal::GetErrorString(nErr);
                    if (aErr.Len())
                    {
                        rStrings.push_back(ScTypedStrData(aErr));
                        continue;
                    }
                }
                else
                    nValue = pFC->GetValue();
            }
            break;
            default:
                ;
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

        rStrings.push_back(ScTypedStrData(aString, nValue, ScTypedStrData::Value));
    }

    rHasDates = bHasDates;
}

//
// GetDataEntries - Strings from continuous Section around nRow
//

// DATENT_MAX      - max. number of entries in list for auto entry
// DATENT_SEARCH   - max. number of cells that get transparent - new: only count Strings
#define DATENT_MAX      200
#define DATENT_SEARCH   2000


bool ScColumn::GetDataEntries(SCROW nStartRow, std::set<ScTypedStrData>& rStrings, bool bLimit)
{
    sal_Bool bFound = false;
    SCSIZE nThisIndex;
    sal_Bool bThisUsed = Search( nStartRow, nThisIndex );
    String aString;
    sal_uInt16 nCells = 0;

    // The limitation to neighbouring cells (without gaps) is not wanted anymore
    // (Featurecommission for 5.1), search upwards/downwards instead so that
    // nearby cell are cought at least first.
    // TODO: Compare distances of cell numbers? Performance??

    SCSIZE nUpIndex = nThisIndex;   // Points after the row
    SCSIZE nDownIndex = nThisIndex; // Points to the row
    if (bThisUsed)
        ++nDownIndex;               // Skip starting row

    while ( nUpIndex || nDownIndex < maItems.size() )
    {
        if ( nUpIndex ) // Up
        {
            ScBaseCell* pCell = maItems[nUpIndex-1].pCell;
            CellType eType = pCell->GetCellType();
            if (eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT) // Only Strings are of interest
            {
                if (eType == CELLTYPE_STRING)
                    aString = ((ScStringCell*)pCell)->GetString();
                else
                    aString = ((ScEditCell*)pCell)->GetString();

                bool bInserted = rStrings.insert(ScTypedStrData(aString)).second;
                if (bInserted && bLimit && rStrings.size() >= DATENT_MAX)
                    break; // Maximum reached
                bFound = true;

                if ( bLimit )
                    if (++nCells >= DATENT_SEARCH)
                        break; // Searched enough
            }
            --nUpIndex;
        }

        if ( nDownIndex < maItems.size() ) // Down
        {
            ScBaseCell* pCell = maItems[nDownIndex].pCell;
            CellType eType = pCell->GetCellType();
            if (eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT) // Only Strings are of interest
            {
                if (eType == CELLTYPE_STRING)
                    aString = ((ScStringCell*)pCell)->GetString();
                else
                    aString = ((ScEditCell*)pCell)->GetString();

                bool bInserted = rStrings.insert(ScTypedStrData(aString)).second;
                if (bInserted && bLimit && rStrings.size() >= DATENT_MAX)
                    break; // Maximum reached
                bFound = true;

                if ( bLimit )
                    if (++nCells >= DATENT_SEARCH)
                        break; // Searched enough
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
            while ( nIndex<maItems.size() && maItems[nIndex].nRow<=nBottom )
            {
                if ( maItems[nIndex].pCell->GetCellType() == CELLTYPE_FORMULA )
                {
                    ScFormulaCell* pFormula = (ScFormulaCell*)maItems[nIndex].pCell;
                    if (pFormula->IsValue())
                    {
                        double nVal = pFormula->GetValue();
                        maItems[nIndex].pCell = new ScValueCell( nVal );
                    }
                    else
                    {
                        String aString = pFormula->GetString();
                        maItems[nIndex].pCell = new ScStringCell( aString );
                    }
                    delete pFormula;

                    SetTextWidth(maItems[nIndex].nRow, TEXTWIDTH_DIRTY);
                    CellStorageModified();
                }
                ++nIndex;
            }
        }

        pPattern = aAttrIter.Next( nTop, nBottom );
    }
}


void ScColumn::SetError( SCROW nRow, const sal_uInt16 nError)
{
    if (ValidRow(nRow))
    {
        ScFormulaCell* pCell = new ScFormulaCell
            ( pDocument, ScAddress( nCol, nRow, nTab ) );
        pCell->SetErrCode( nError );
        Insert( nRow, pCell );
    }
}

void ScColumn::SetRawString( SCROW nRow, const OUString& rStr )
{
    if (!ValidRow(nRow))
        return;

    ScBaseCell* pCell = new ScStringCell(rStr);
    Insert(nRow, pCell);
}

void ScColumn::SetValue( SCROW nRow, const double& rVal)
{
    if (ValidRow(nRow))
    {
        ScBaseCell* pCell = new ScValueCell(rVal);
        Insert( nRow, pCell );
    }
}


void ScColumn::GetString( SCROW nRow, OUString& rString ) const
{
    SCSIZE  nIndex;
    Color* pColor;
    if (Search(nRow, nIndex))
    {
        ScRefCellValue aCell;
        aCell.assign(*maItems[nIndex].pCell);

        // ugly hack for ordering problem with GetNumberFormat and missing inherited formats
        if(aCell.meType == CELLTYPE_FORMULA)
            aCell.mpFormula->MaybeInterpret();

        sal_uLong nFormat = GetNumberFormat( nRow );
        ScCellFormat::GetString(aCell, nFormat, rString, &pColor, *(pDocument->GetFormatTable()), pDocument);
    }
    else
        rString = EMPTY_OUSTRING;
}

const OUString* ScColumn::GetStringCell( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (!Search(nRow, nIndex))
        return NULL;

    const ScBaseCell* pCell = maItems[nIndex].pCell;
    if (pCell->GetCellType() != CELLTYPE_STRING)
        return NULL;

    return static_cast<const ScStringCell*>(pCell)->GetStringPtr();
}

double* ScColumn::GetValueCell( SCROW nRow )
{
    SCSIZE  nIndex;
    if (!Search(nRow, nIndex))
        return NULL;

    ScBaseCell* pCell = maItems[nIndex].pCell;
    if (pCell->GetCellType() != CELLTYPE_VALUE)
        return NULL;

    return static_cast<ScValueCell*>(pCell)->GetValuePtr();
}

void ScColumn::GetInputString( SCROW nRow, OUString& rString ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
    {
        ScRefCellValue aCell;
        aCell.assign(*maItems[nIndex].pCell);
        sal_uLong nFormat = GetNumberFormat( nRow );
        ScCellFormat::GetInputString(aCell, nFormat, rString, *(pDocument->GetFormatTable()), pDocument);
    }
    else
        rString = OUString();
}


double ScColumn::GetValue( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = maItems[nIndex].pCell;
        switch (pCell->GetCellType())
        {
            case CELLTYPE_VALUE:
                return ((ScValueCell*)pCell)->GetValue();

            case CELLTYPE_FORMULA:
                {
                    if (((ScFormulaCell*)pCell)->IsValue())
                        return ((ScFormulaCell*)pCell)->GetValue();
                    else
                        return 0.0;
                }

            default:
                return 0.0;
        }
    }
    return 0.0;
}

const EditTextObject* ScColumn::GetEditText( SCROW nRow ) const
{
    SCSIZE nIndex;
    if (!Search(nRow, nIndex))
        return NULL;

    const ScBaseCell* pCell = maItems[nIndex].pCell;
    if (pCell->GetCellType() != CELLTYPE_EDIT)
        return NULL;

    const ScEditCell* pEditCell = static_cast<const ScEditCell*>(pCell);
    return pEditCell->GetData();
}

void ScColumn::RemoveEditTextCharAttribs( SCROW nRow, const ScPatternAttr& rAttr )
{
    SCSIZE nIndex;
    if (!Search(nRow, nIndex))
        return;

    ScBaseCell* pCell = maItems[nIndex].pCell;
    if (pCell->GetCellType() != CELLTYPE_EDIT)
        return;

    ScEditCell* pEditCell = static_cast<ScEditCell*>(pCell);
    pEditCell->RemoveCharAttribs(rAttr);
}

void ScColumn::GetFormula( SCROW nRow, OUString& rFormula ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = maItems[nIndex].pCell;
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
            ((ScFormulaCell*)pCell)->GetFormula( rFormula );
        else
            rFormula = OUString();
    }
    else
        rFormula = OUString();
}

const ScTokenArray* ScColumn::GetFormulaTokens( SCROW nRow ) const
{
    const ScFormulaCell* pCell = FetchFormulaCell(nRow);
    if (!pCell)
        return NULL;

    return pCell->GetCode();
}

const ScFormulaCell* ScColumn::GetFormulaCell( SCROW nRow ) const
{
    return FetchFormulaCell(nRow);
}

ScFormulaCell* ScColumn::GetFormulaCell( SCROW nRow )
{
    return const_cast<ScFormulaCell*>(FetchFormulaCell(nRow));
}

CellType ScColumn::GetCellType( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
        return maItems[nIndex].pCell->GetCellType();
    return CELLTYPE_NONE;
}

SCSIZE ScColumn::GetCellCount() const
{
    return maItems.size();
}

sal_uInt16 ScColumn::GetErrCode( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
    {
        ScBaseCell* pCell = maItems[nIndex].pCell;
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
            return ((ScFormulaCell*)pCell)->GetErrCode();
    }
    return 0;
}


bool ScColumn::HasStringData( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
        return (maItems[nIndex].pCell)->HasStringData();
    return false;
}


bool ScColumn::HasValueData( SCROW nRow ) const
{
    SCSIZE  nIndex;
    if (Search(nRow, nIndex))
        return (maItems[nIndex].pCell)->HasValueData();
    return false;
}

/**
 * Return true if there is a string or editcell in the range
 */
bool ScColumn::HasStringCells( SCROW nStartRow, SCROW nEndRow ) const
{
    if ( !maItems.empty() )
    {
        SCSIZE nIndex;
        Search( nStartRow, nIndex );
        while ( nIndex < maItems.size() && maItems[nIndex].nRow <= nEndRow )
        {
            CellType eType = maItems[nIndex].pCell->GetCellType();
            if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
                return sal_True;
            ++nIndex;
        }
    }
    return false;
}


sal_Int32 ScColumn::GetMaxStringLen( SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const
{
    sal_Int32 nStringLen = 0;
    if ( !maItems.empty() )
    {
        OUString aString;
        OString aOString;
        bool bIsOctetTextEncoding = rtl_isOctetTextEncoding( eCharSet);
        SvNumberFormatter* pNumFmt = pDocument->GetFormatTable();
        SCSIZE nIndex;
        SCROW nRow;
        Search( nRowStart, nIndex );
        while ( nIndex < maItems.size() && (nRow = maItems[nIndex].nRow) <= nRowEnd )
        {
            ScRefCellValue aCell;
            aCell.assign(*maItems[nIndex].pCell);
            Color* pColor;
            sal_uLong nFormat = (sal_uLong) ((SfxUInt32Item*) GetAttr(
                nRow, ATTR_VALUE_FORMAT ))->GetValue();
            ScCellFormat::GetString(aCell, nFormat, aString, &pColor, *pNumFmt, pDocument);
            sal_Int32 nLen;
            if (bIsOctetTextEncoding)
            {
                if (!aString.convertToString( &aOString, eCharSet,
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
                nLen = aString.getLength() * sizeof(sal_Unicode);
            if ( nStringLen < nLen)
                nStringLen = nLen;
            nIndex++;
        }
    }
    return nStringLen;
}


xub_StrLen ScColumn::GetMaxNumberStringLen(
    sal_uInt16& nPrecision, SCROW nRowStart, SCROW nRowEnd ) const
{
    xub_StrLen nStringLen = 0;
    nPrecision = 0;

    if ( !maItems.empty() )
    {
        OUString aString;
        String aSep;
        SvNumberFormatter* pNumFmt = pDocument->GetFormatTable();
        sal_uInt16 nMaxGeneralPrecision = pDocument->GetDocOptions().GetStdPrecision();
        // Limit the decimals passed to doubleToUString().
        // Also, the dBaseIII maximum precision is 15.
        if (nMaxGeneralPrecision > 15)
            nMaxGeneralPrecision = 15;
        bool bHaveSigned = false;
        SCSIZE nIndex;
        SCROW nRow;
        Search( nRowStart, nIndex );
        while ( nIndex < maItems.size() && (nRow = maItems[nIndex].nRow) <= nRowEnd )
        {
            ScRefCellValue aCell;
            aCell.assign(*maItems[nIndex].pCell);
            CellType eType = aCell.meType;
            if ( eType == CELLTYPE_VALUE || (eType == CELLTYPE_FORMULA
                    && aCell.mpFormula->IsValue()) )
            {
                do
                {
                    sal_uInt16 nCellPrecision = nMaxGeneralPrecision;
                    if (eType == CELLTYPE_FORMULA)
                    {
                        // Limit unformatted formula cell precision to precision
                        // encountered so far, if any, otherwise we'd end up with 15 just
                        // because of =1/3 ...  If no precision yet then arbitrarily limit
                        // to a maximum of 4 unless a maximum general precision is set.
                        if (nPrecision)
                            nCellPrecision = nPrecision;
                        else
                            nCellPrecision = (nMaxGeneralPrecision >= 15) ? 4 : nMaxGeneralPrecision;
                    }

                    double fVal = aCell.getValue();
                    if (!bHaveSigned && fVal < 0.0)
                        bHaveSigned = true;

                    sal_uInt16 nPrec;
                    sal_uLong nFormat = (sal_uLong) ((SfxUInt32Item*) GetAttr(
                                nRow, ATTR_VALUE_FORMAT ))->GetValue();
                    if (nFormat % SV_COUNTRY_LANGUAGE_OFFSET)
                    {
                        aSep = pNumFmt->GetFormatDecimalSep(nFormat);
                        ScCellFormat::GetInputString(aCell, nFormat, aString, *pNumFmt, pDocument);
                        const SvNumberformat* pEntry = pNumFmt->GetEntry( nFormat );
                        if (pEntry)
                        {
                            bool bThousand, bNegRed;
                            sal_uInt16 nLeading;
                            pEntry->GetFormatSpecialInfo(bThousand, bNegRed, nPrec, nLeading);
                        }
                        else
                            nPrec = pNumFmt->GetFormatPrecision( nFormat );
                    }
                    else
                    {
                        if (nPrecision >= nMaxGeneralPrecision)
                            break;      // early bail out for nothing changes here

                        if (!fVal)
                        {
                            // 0 doesn't change precision, but set a maximum length if none yet.
                            if (!nStringLen)
                                nStringLen = 1;
                            break;
                        }

                        // Simple number string with at most 15 decimals and trailing
                        // decimal zeros eliminated.
                        aSep = ".";
                        aString = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_F, nCellPrecision, '.', true);
                        nPrec = SvNumberFormatter::UNLIMITED_PRECISION;
                    }

                    sal_Int32 nLen = aString.getLength();
                    if (nLen <= 0)
                        // Ignore empty string.
                        break;

                    if (nPrec == SvNumberFormatter::UNLIMITED_PRECISION && nPrecision < nMaxGeneralPrecision)
                    {
                        if (nFormat % SV_COUNTRY_LANGUAGE_OFFSET)
                        {
                            // For some reason we couldn't obtain a precision from the
                            // format, retry with simple number string.
                            aSep = ".";
                            aString = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_F, nCellPrecision, '.', true);
                            nLen = aString.getLength();
                        }
                        sal_Int32 nSep = aString.indexOf( aSep);
                        if (nSep != -1)
                            nPrec = aString.getLength() - nSep - 1;

                    }

                    if (nPrec != SvNumberFormatter::UNLIMITED_PRECISION && nPrec > nPrecision)
                        nPrecision = nPrec;

                    if ( nPrecision )
                    {   // less than nPrecision in string => widen it
                        // more => shorten it
                        sal_Int32 nTmp = aString.indexOf( aSep );
                        if ( nTmp == -1 )
                            nLen += nPrecision + aSep.Len();
                        else
                        {
                            nTmp = aString.getLength() - (nTmp + aSep.Len());
                            if ( nTmp != nPrecision )
                                nLen += nPrecision - nTmp;
                                // nPrecision > nTmp : nLen + Diff
                                // nPrecision < nTmp : nLen - Diff
                        }
                    }

                    // Enlarge for sign if necessary. Bear in mind that
                    // GetMaxNumberStringLen() is for determining dBase decimal field width
                    // and precision where the overall field width must include the sign.
                    // Fitting -1 into "#.##" (width 4, 2 decimals) does not work.
                    if (bHaveSigned && fVal >= 0.0)
                        ++nLen;

                    if ( nStringLen < nLen )
                        nStringLen = nLen;

                } while (0);
            }
            nIndex++;
        }
    }
    return nStringLen;
}

namespace {

struct CellGroupSetter : std::unary_function<ColEntry, void>
{
    ScFormulaCellGroupRef mxGroup;
public:
    CellGroupSetter(const ScFormulaCellGroupRef& xGroup) : mxGroup(xGroup) {}

    void operator() (ColEntry& rEntry)
    {
        if (rEntry.pCell && rEntry.pCell->GetCellType() == CELLTYPE_FORMULA)
            static_cast<ScFormulaCell*>(rEntry.pCell)->SetCellGroup(mxGroup);
    }
};

}

// Very[!] slow way to look for and merge contiguous runs
// of similar formulae into a formulagroup
void ScColumn::RebuildFormulaGroups()
{
    if ( maItems.empty() || !mbDirtyGroups )
        return;

    // clear double groups
    std::for_each(maDoubles.begin(), maDoubles.end(), ScDeleteObjectByPtr<ColDoubleEntry>());
    maDoubles.clear();

    // clear previous groups
    ScFormulaCellGroupRef xNone;
    std::for_each(maItems.begin(), maItems.end(), CellGroupSetter(xNone));
    maFnGroups.clear();

    // re-build groups
    ColDoubleEntry *pLastDouble = NULL;
    for (size_t i = 1; i < maItems.size(); i++)
    {
        ColEntry &rCur = maItems[ i ];
        ColEntry &rPrev = maItems[ i - 1 ];
        if ( ( rPrev.nRow != rCur.nRow - 1 ) ||                        // not contiguous
             !rCur.pCell || !rPrev.pCell ||                            // paranoia
             rCur.pCell->GetCellType() != rPrev.pCell->GetCellType() ) // same type
        {
            // Non-contiguous cell detected. Break the series.
            pLastDouble = NULL;
            continue;
        }

        // collate doubles
        if ( rCur.pCell->GetCellType() == CELLTYPE_VALUE )
        {
            if ( !pLastDouble )
            {
                pLastDouble = new ColDoubleEntry();
                pLastDouble->mnStart = rPrev.nRow;
                pLastDouble->maData.push_back(
                        static_cast< ScValueCell * >( rPrev.pCell )->GetValue() );
                maDoubles.push_back( pLastDouble );
            }
            pLastDouble->maData.push_back(
                        static_cast< ScValueCell * >( rCur.pCell )->GetValue() );
            continue;
        }

        if ( rCur.pCell->GetCellType() != CELLTYPE_FORMULA )
            continue;

        // see if these formula tokens are identical.
        ScFormulaCell *pCur = static_cast< ScFormulaCell *>( rCur.pCell );
        ScFormulaCell *pPrev = static_cast< ScFormulaCell *>( rPrev.pCell );

        ScFormulaCell::CompareState eCompState = pPrev->CompareByTokenArray(pCur);
        if (eCompState == ScFormulaCell::NotEqual)
        {
            // different formula tokens.
            pCur->SetCellGroup( xNone );
            continue;
        }

        ScFormulaCellGroupRef xGroup = pPrev->GetCellGroup();
        if (!xGroup)
        {
            // create a new group ...
            xGroup.reset(new ScFormulaCellGroup);
            xGroup->mnStart = rPrev.nRow;
            xGroup->mbInvariant = eCompState == ScFormulaCell::EqualInvariant;
            xGroup->mnLength = 2;

            maFnGroups.push_back( xGroup );

            pCur->SetCellGroup( xGroup );
            pPrev->SetCellGroup( xGroup );
        }
        else
        {
            // existing group. extend its length.
            pCur->SetCellGroup( xGroup );
            xGroup->mnLength++;
        }
    }

#if OSL_DEBUG_LEVEL > 0
    if ( maDoubles.size() + maFnGroups.size() > 0 )
    {
        OUString aStr;
        fprintf( stderr, "column %2d has %2d double span(s): ", (int)nCol, (int)maDoubles.size() );
        for (std::vector< ColDoubleEntry *>::iterator it = maDoubles.begin();
             it != maDoubles.end(); ++it )
        {
            ScRange aDoubleRange( nCol, (*it)->mnStart, nTab,
                                  nCol, (*it)->mnStart + (*it)->maData.size() - 1, nTab );
            aDoubleRange.Format( aStr, SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW, pDocument );
            fprintf( stderr, "%s, ", OUStringToOString( aStr, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        fprintf( stderr, "\n" );

        fprintf( stderr, "column %2d has %2d formula span(s): ", (int)nCol, (int)maFnGroups.size() );
        for (std::vector< ScFormulaCellGroupRef>::iterator it = maFnGroups.begin();
             it != maFnGroups.end(); ++it )
        {
            ScRange aDoubleRange( nCol, (*it)->mnStart, nTab,
                                  nCol, (*it)->mnStart + (*it)->mnLength - 1, nTab );
            aDoubleRange.Format( aStr, SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW, pDocument );
            fprintf( stderr, "%s, ", OUStringToOString( aStr, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        fprintf( stderr, "\n" );
    }
#endif

    mbDirtyGroups = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
