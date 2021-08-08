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

#include <unotools/textsearch.hxx>
#include <com/sun/star/util/SearchResult.hpp>
#include <svl/srchitem.hxx>
#include <editeng/editobj.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <table.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <markdata.hxx>
#include <editutil.hxx>
#include <postit.hxx>

namespace {

bool lcl_GetTextWithBreaks( const EditTextObject& rData, ScDocument* pDoc, OUString& rVal )
{
    //  true = more than 1 paragraph

    EditEngine& rEngine = pDoc->GetEditEngine();
    rEngine.SetText(rData);
    rVal = rEngine.GetText();
    return ( rEngine.GetParagraphCount() > 1 );
}

}

bool ScTable::SearchCell(const SvxSearchItem& rSearchItem, SCCOL nCol, sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow,
                         const ScMarkData& rMark, OUString& rUndoStr, ScDocument* pUndoDoc)
{
    if ( !IsColRowValid( nCol, nRow ) )
        return false;

    bool    bFound = false;
    bool    bDoSearch = true;
    bool    bDoBack = rSearchItem.GetBackward();
    bool    bSearchFormatted = rSearchItem.IsSearchFormatted();

    OUString  aString;
    ScRefCellValue aCell;
    if (rSearchItem.GetSelection())
        bDoSearch = rMark.IsCellMarked(nCol, nRow);

    if (!bDoSearch)
        return false;

    ScPostIt* pNote;
    if (rSearchItem.GetCellType() == SvxSearchCellType::NOTE)
    {
        pNote = aCol[nCol].GetCellNote(rBlockPos, nRow);
        if (!pNote)
            return false;
    }
    else
    {
        aCell = aCol[nCol].GetCellValue(rBlockPos, nRow);
        if (aCell.isEmpty())
            return false;
        pNote = nullptr;
    }

    bool bMultiLine = false;
    CellType eCellType = aCell.meType;
    switch (rSearchItem.GetCellType())
    {
        case SvxSearchCellType::FORMULA:
        {
            if ( eCellType == CELLTYPE_FORMULA )
                aCell.mpFormula->GetFormula(aString, rDocument.GetGrammar());
            else if ( eCellType == CELLTYPE_EDIT )
                bMultiLine = lcl_GetTextWithBreaks(*aCell.mpEditText, &rDocument, aString);
            else
            {
                if( !bSearchFormatted )
                    aCol[nCol].GetInputString( rBlockPos, nRow, aString );
                else
                    aCol[nCol].GetString( rBlockPos, nRow, aString );
            }
            break;
        }
        case SvxSearchCellType::VALUE:
            if ( eCellType == CELLTYPE_EDIT )
                bMultiLine = lcl_GetTextWithBreaks(*aCell.mpEditText, &rDocument, aString);
            else
            {
                if( !bSearchFormatted )
                    aCol[nCol].GetInputString( rBlockPos, nRow, aString );
                else
                    aCol[nCol].GetString( rBlockPos, nRow, aString );
            }
            break;
        case SvxSearchCellType::NOTE:
        {
            if (pNote)
            {
                aString = pNote->GetText();
                bMultiLine = pNote->HasMultiLineText();
            }
            break;
        }
        default:
            break;
    }
    sal_Int32 nStart = 0;
    sal_Int32 nEnd = aString.getLength();
    css::util::SearchResult aSearchResult;
    if (pSearchText)
    {
        if ( bDoBack )
        {
            sal_Int32 nTemp=nStart; nStart=nEnd; nEnd=nTemp;
            bFound = pSearchText->SearchBackward(aString, &nStart, &nEnd, &aSearchResult);
            // change results to definition before 614:
            --nEnd;
        }
        else
        {
            bFound = pSearchText->SearchForward(aString, &nStart, &nEnd, &aSearchResult);
            // change results to definition before 614:
            --nEnd;
        }

        if (bFound && rSearchItem.GetWordOnly())
            bFound = (nStart == 0 && nEnd == aString.getLength() - 1);
    }
    else
    {
        OSL_FAIL("pSearchText == NULL");
        return bFound;
    }

    if (!bFound)
        return false;
    if ( rSearchItem.GetCommand() != SvxSearchCmd::REPLACE
         && rSearchItem.GetCommand() != SvxSearchCmd::REPLACE_ALL )
        return bFound;

    if (!IsBlockEditable(nCol, nRow, nCol, nRow))
        return bFound;

    ScMatrixMode cMatrixFlag = ScMatrixMode::NONE;

    // Don't split the matrix, only replace Matrix formulas
    if (eCellType == CELLTYPE_FORMULA)
    {
        cMatrixFlag = aCell.mpFormula->GetMatrixFlag();
        if(cMatrixFlag == ScMatrixMode::Reference)
            return bFound;
    }
    // No UndoDoc => Matrix not restorable => don't replace
    if (cMatrixFlag != ScMatrixMode::NONE && !pUndoDoc)
        return bFound;

    if ( cMatrixFlag == ScMatrixMode::NONE && rSearchItem.GetCommand() == SvxSearchCmd::REPLACE )
        rUndoStr = aString;
    else if (pUndoDoc)
    {
        ScAddress aAdr( nCol, nRow, nTab );
        aCell.commit(*pUndoDoc, aAdr);
    }

    bool bRepeat = !rSearchItem.GetWordOnly();
    do
    {
        //  don't continue search if the found text is empty,
        //  otherwise it would never stop (#35410#)
        if ( nEnd < nStart )
            bRepeat = false;

        OUString sReplStr = rSearchItem.GetReplaceString();
        if (rSearchItem.GetRegExp())
        {
            pSearchText->ReplaceBackReferences( sReplStr, aString, aSearchResult );
            OUStringBuffer aStrBuffer(aString);
            aStrBuffer.remove(nStart, nEnd-nStart+1);
            aStrBuffer.insert(nStart, sReplStr);
            aString = aStrBuffer.makeStringAndClear();
        }
        else
        {
            OUStringBuffer aStrBuffer(aString);
            aStrBuffer.remove(nStart, nEnd-nStart+1);
            aStrBuffer.insert(nStart, rSearchItem.GetReplaceString());
            aString = aStrBuffer.makeStringAndClear();
        }

        //  Adjust index
        if (bDoBack)
        {
            nEnd = nStart;
            nStart = 0;
        }
        else
        {
            nStart = nStart + sReplStr.getLength();
            nEnd = aString.getLength();
        }

        //  continue search ?
        if (bRepeat)
        {
            if ( rSearchItem.GetCommand() != SvxSearchCmd::REPLACE_ALL || nStart >= nEnd )
                bRepeat = false;
            else if (bDoBack)
            {
                sal_Int32 nTemp=nStart; nStart=nEnd; nEnd=nTemp;
                bRepeat = pSearchText->SearchBackward(aString, &nStart, &nEnd, &aSearchResult);
                // change results to definition before 614:
                --nEnd;
            }
            else
            {
                bRepeat = pSearchText->SearchForward(aString, &nStart, &nEnd, &aSearchResult);
                // change results to definition before 614:
                --nEnd;
            }
        }
    }
    while (bRepeat);
    if (rSearchItem.GetCellType() == SvxSearchCellType::NOTE)
    {
        // NB: rich text format is lost.
        // This is also true of Cells.
        if (pNote)
            pNote->SetText( ScAddress( nCol, nRow, nTab ), aString );
    }
    else if ( cMatrixFlag != ScMatrixMode::NONE )
    {   // don't split Matrix
        if ( aString.getLength() > 2 )
        {   // remove {} here so that "{=" can be replaced by "{=..."
            if ( aString[ aString.getLength()-1 ] == '}' )
                aString = aString.copy( 0, aString.getLength()-1 );
            if ( aString[0] == '{' )
                aString = aString.copy( 1 );
        }
        ScAddress aAdr( nCol, nRow, nTab );
        ScFormulaCell* pFCell = new ScFormulaCell( rDocument, aAdr,
            aString, rDocument.GetGrammar(), cMatrixFlag );
        SCCOL nMatCols;
        SCROW nMatRows;
        aCell.mpFormula->GetMatColsRows(nMatCols, nMatRows);
        pFCell->SetMatColsRows( nMatCols, nMatRows );
        aCol[nCol].SetFormulaCell(nRow, pFCell);
    }
    else if ( bMultiLine && aString.indexOf('\n') != -1 )
    {
        ScFieldEditEngine& rEngine = rDocument.GetEditEngine();
        rEngine.SetTextCurrentDefaults(aString);
        SetEditText(nCol, nRow, rEngine.CreateTextObject());
    }
    else
        aCol[nCol].SetString(nRow, nTab, aString, rDocument.GetAddressConvention());
    // pCell is invalid now (deleted)
    aCol[nCol].InitBlockPosition( rBlockPos ); // invalidate also the cached position

    return bFound;
}

void ScTable::SkipFilteredRows(SCROW& rRow, SCROW& rLastNonFilteredRow, bool bForward)
{
    if (bForward)
    {
        // forward search

        if (rRow <= rLastNonFilteredRow)
            return;

        SCROW nLastRow = rRow;
        if (RowFiltered(rRow, nullptr, &nLastRow))
            // move to the first non-filtered row.
            rRow = nLastRow + 1;
        else
            // record the last non-filtered row to avoid checking
            // the filtered state for each and every row.
            rLastNonFilteredRow = nLastRow;
    }
    else
    {
        // backward search

        if (rRow >= rLastNonFilteredRow)
            return;

        SCROW nFirstRow = rRow;
        if (RowFiltered(rRow, &nFirstRow))
            // move to the first non-filtered row.
            rRow = nFirstRow - 1;
        else
            // record the last non-filtered row to avoid checking
            // the filtered state for each and every row.
            rLastNonFilteredRow = nFirstRow;
    }
}

bool ScTable::Search(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                     const ScMarkData& rMark, OUString& rUndoStr, ScDocument* pUndoDoc)
{
    SCCOL nLastCol;
    SCROW nLastRow;
    if (rSearchItem.GetCellType() == SvxSearchCellType::NOTE)
        GetCellArea( nLastCol, nLastRow);
    else
        GetLastDataPos(nLastCol, nLastRow);
    return Search(rSearchItem, rCol, rRow, nLastCol, nLastRow, rMark, rUndoStr, pUndoDoc);
}

bool ScTable::Search(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                     SCCOL nLastCol, SCROW nLastRow,
                     const ScMarkData& rMark, OUString& rUndoStr, ScDocument* pUndoDoc)
{
    bool bFound = false;
    bool bAll =  (rSearchItem.GetCommand() == SvxSearchCmd::FIND_ALL)
               ||(rSearchItem.GetCommand() == SvxSearchCmd::REPLACE_ALL);
    SCCOL nCol = rCol;
    SCROW nRow = rRow;

    bool bSkipFiltered = !rSearchItem.IsSearchFiltered();
    bool bSearchNotes = (rSearchItem.GetCellType() == SvxSearchCellType::NOTE);
    // We need to cache sc::ColumnBlockConstPosition per each column.
    std::vector< sc::ColumnBlockConstPosition > blockPos( nLastCol + 1 );
    for( SCCOL i = 0; i <= nLastCol; ++i )
        aCol[ i ].InitBlockPosition( blockPos[ i ] );
    if (!bAll && rSearchItem.GetBackward())
    {
        SCROW nLastNonFilteredRow = rDocument.MaxRow() + 1;
        if (rSearchItem.GetRowDirection())
        {
            nCol--;
            nCol = std::min(nCol, nLastCol);
            nRow = std::min(nRow, nLastRow);
            while (!bFound && (nRow >= 0))
            {
                if (bSkipFiltered)
                    SkipFilteredRows(nRow, nLastNonFilteredRow, false);

                while (!bFound && (nCol >= 0))
                {
                    bFound = SearchCell(rSearchItem, nCol, blockPos[ nCol ], nRow,
                                        rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                        bool bIsEmpty;
                        do
                        {
                            nCol--;
                            if (nCol >= 0)
                            {
                                if (bSearchNotes)
                                    bIsEmpty = !aCol[nCol].HasCellNotes();
                                else
                                    bIsEmpty = aCol[nCol].IsEmptyData();
                            }
                            else
                                bIsEmpty = true;
                        }
                        while ((nCol >= 0) && bIsEmpty);
                    }
                }
                if (!bFound)
                {
                    nCol = nLastCol;
                    nRow--;
                }
            }
        }
        else
        {
            nRow--;
            nCol = std::min(nCol, nLastCol);
            nRow = std::min(nRow, nLastRow);
            while (!bFound && (nCol >= 0))
            {
                while (!bFound && (nRow >= 0))
                {
                    if (bSkipFiltered)
                        SkipFilteredRows(nRow, nLastNonFilteredRow, false);

                    bFound = SearchCell(rSearchItem, nCol, blockPos[ nCol ],
                                        nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                        if (bSearchNotes)
                        {
                            /* TODO: can we look for the previous cell note instead? */
                            --nRow;
                        }
                        else
                        {
                            if (!aCol[nCol].GetPrevDataPos(nRow))
                                nRow = -1;
                        }
                    }
                }
                if (!bFound)
                {
                    // Not found in this column.  Move to the next column.
                    bool bIsEmpty;
                    nRow = nLastRow;
                    nLastNonFilteredRow = rDocument.MaxRow() + 1;
                    do
                    {
                        nCol--;
                        if (nCol >= 0)
                        {
                            if (bSearchNotes)
                                bIsEmpty = !aCol[nCol].HasCellNotes();
                            else
                                bIsEmpty = aCol[nCol].IsEmptyData();
                        }
                        else
                            bIsEmpty = true;
                    }
                    while ((nCol >= 0) && bIsEmpty);
                }
            }
        }
    }
    else
    {
        SCROW nLastNonFilteredRow = -1;
        if (rSearchItem.GetRowDirection())
        {
            nCol++;
            while (!bFound && (nRow <= nLastRow))
            {
                if (bSkipFiltered)
                    SkipFilteredRows(nRow, nLastNonFilteredRow, true);

                while (!bFound && (nCol <= nLastCol))
                {
                    bFound = SearchCell(rSearchItem, nCol, blockPos[ nCol ],
                                        nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                        nCol++;
                        while ((nCol <= nLastCol) &&
                                (bSearchNotes ? !aCol[nCol].HasCellNotes() : aCol[nCol].IsEmptyData()))
                            nCol++;
                    }
                }
                if (!bFound)
                {
                    nCol = 0;
                    nRow++;
                }
            }
        }
        else
        {
            nRow++;
            while (!bFound && (nCol <= nLastCol))
            {
                while (!bFound && (nRow <= nLastRow))
                {
                    if (bSkipFiltered)
                        SkipFilteredRows(nRow, nLastNonFilteredRow, true);

                    bFound = SearchCell(rSearchItem, nCol, blockPos[ nCol ],
                                        nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                        if (bSearchNotes)
                        {
                            /* TODO: can we look for the next cell note instead? */
                            ++nRow;
                        }
                        else
                        {
                            if (!aCol[nCol].GetNextDataPos(nRow))
                                nRow = rDocument.MaxRow() + 1;
                        }
                    }
                }
                if (!bFound)
                {
                    // Not found in this column.  Move to the next column.
                    nRow = 0;
                    nLastNonFilteredRow = -1;
                    nCol++;
                    while ((nCol <= nLastCol) &&
                            (bSearchNotes ? !aCol[nCol].HasCellNotes() : aCol[nCol].IsEmptyData()))
                        nCol++;
                }
            }
        }
    }
    if (bFound)
    {
        rCol = nCol;
        rRow = nRow;
    }
    return bFound;
}

bool ScTable::SearchAll(const SvxSearchItem& rSearchItem, const ScMarkData& rMark,
                        ScRangeList& rMatchedRanges, OUString& rUndoStr, ScDocument* pUndoDoc)
{
    bool bFound = true;
    SCCOL nCol = 0;
    SCROW nRow = -1;
    bool bEverFound = false;

    SCCOL nLastCol;
    SCROW nLastRow;
    if (rSearchItem.GetCellType() == SvxSearchCellType::NOTE)
        GetCellArea( nLastCol, nLastRow);
    else
        GetLastDataPos(nLastCol, nLastRow);

    do
    {
        bFound = Search(rSearchItem, nCol, nRow, nLastCol, nLastRow, rMark, rUndoStr, pUndoDoc);
        if (bFound)
        {
            bEverFound = true;
            rMatchedRanges.Join(ScRange(nCol, nRow, nTab));
        }
    }
    while (bFound);

    return bEverFound;
}

void ScTable::UpdateSearchItemAddressForReplace( const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow )
{
    if (rSearchItem.GetBackward())
    {
        if (rSearchItem.GetRowDirection())
            rCol += 1;
        else
            rRow += 1;
    }
    else
    {
        if (rSearchItem.GetRowDirection())
            rCol -= 1;
        else
            rRow -= 1;
    }
}

bool ScTable::Replace(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                      const ScMarkData& rMark, OUString& rUndoStr, ScDocument* pUndoDoc)
{
    SCCOL nCol = rCol;
    SCROW nRow = rRow;

    UpdateSearchItemAddressForReplace( rSearchItem, nCol, nRow );
    bool bFound = Search(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
    if (bFound)
    {
        rCol = nCol;
        rRow = nRow;
    }
    return bFound;
}

bool ScTable::ReplaceAll(
    const SvxSearchItem& rSearchItem, const ScMarkData& rMark, ScRangeList& rMatchedRanges,
    OUString& rUndoStr, ScDocument* pUndoDoc)
{
    SCCOL nCol = 0;
    SCROW nRow = -1;

    SCCOL nLastCol;
    SCROW nLastRow;
    if (rSearchItem.GetCellType() == SvxSearchCellType::NOTE)
        GetCellArea( nLastCol, nLastRow);
    else
        GetLastDataPos(nLastCol, nLastRow);

    // tdf#92160 - columnar replace is faster, and more memory efficient.
    SvxSearchItem aCopyItem(rSearchItem);
    aCopyItem.SetRowDirection(false);

    bool bEverFound = false;
    while (true)
    {
        bool bFound = Search(aCopyItem, nCol, nRow, nLastCol, nLastRow, rMark, rUndoStr, pUndoDoc);

        if (bFound)
        {
            bEverFound = true;
            rMatchedRanges.Join(ScRange(nCol, nRow, nTab));
        }
        else
            break;
    }
    return bEverFound;
}

bool ScTable::SearchStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                          const ScMarkData& rMark)
{
    const ScStyleSheet* pSearchStyle = static_cast<const ScStyleSheet*>(
                                        rDocument.GetStyleSheetPool()->Find(
                                        rSearchItem.GetSearchString(), SfxStyleFamily::Para ));

    SCCOL nCol = rCol;
    SCROW nRow = rRow;
    bool bFound = false;

    bool bSelect = rSearchItem.GetSelection();
    bool bRows = rSearchItem.GetRowDirection();
    bool bBack = rSearchItem.GetBackward();
    short nAdd = bBack ? -1 : 1;

    if (bRows)                                      // by row
    {
        if ( !IsColValid( nCol ) )
        {
            SAL_WARN( "sc.core", "SearchStyle: bad column " << nCol);
            return false;
        }
        nRow += nAdd;
        do
        {
            SCROW nNextRow = aCol[nCol].SearchStyle( nRow, pSearchStyle, bBack, bSelect, rMark );
            if (!ValidRow(nNextRow))
            {
                nRow = bBack ? rDocument.MaxRow() : 0;
                nCol = sal::static_int_cast<SCCOL>( nCol + nAdd );
            }
            else
            {
                nRow = nNextRow;
                bFound = true;
            }
        }
        while ( !bFound && IsColValid( nCol ) );
    }
    else                                    // by column
    {
        SCCOL aColSize = aCol.size();
        std::vector< SCROW > nNextRows ( aColSize );
        SCCOL i;
        for (i=0; i < aColSize; ++i)
        {
            SCROW nSRow = nRow;
            if (bBack)
            {
                if (i>=nCol) --nSRow;
            }
            else
            {
                if (i<=nCol) ++nSRow;
            }
            nNextRows[i] = aCol[i].SearchStyle( nSRow, pSearchStyle, bBack, bSelect, rMark );
        }
        if (bBack)                          // backwards
        {
            nRow = -1;
            for (i = aColSize - 1; i>=0; --i)
                if (nNextRows[i]>nRow)
                {
                    nCol = i;
                    nRow = nNextRows[i];
                    bFound = true;
                }
        }
        else                                // forwards
        {
            nRow = rDocument.MaxRow()+1;
            for (i=0; i < aColSize; ++i)
                if (nNextRows[i]<nRow)
                {
                    nCol = i;
                    nRow = nNextRows[i];
                    bFound = true;
                }
        }
    }

    if (bFound)
    {
        rCol = nCol;
        rRow = nRow;
    }
    return bFound;
}

//TODO: return single Pattern for Undo

bool ScTable::ReplaceStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                           const ScMarkData& rMark, bool bIsUndo)
{
    bool bRet;
    if (bIsUndo)
        bRet = true;
    else
        bRet = SearchStyle(rSearchItem, rCol, rRow, rMark);
    if (bRet)
    {
        const ScStyleSheet* pReplaceStyle = static_cast<const ScStyleSheet*>(
                                        rDocument.GetStyleSheetPool()->Find(
                                        rSearchItem.GetReplaceString(), SfxStyleFamily::Para ));

        if (pReplaceStyle)
            ApplyStyle( rCol, rRow, pReplaceStyle );
        else
        {
            OSL_FAIL("pReplaceStyle==0");
        }
    }

    return bRet;
}

bool ScTable::SearchAllStyle(
    const SvxSearchItem& rSearchItem, const ScMarkData& rMark, ScRangeList& rMatchedRanges)
{
    const ScStyleSheet* pSearchStyle = static_cast<const ScStyleSheet*>(
                                        rDocument.GetStyleSheetPool()->Find(
                                        rSearchItem.GetSearchString(), SfxStyleFamily::Para ));
    bool bSelect = rSearchItem.GetSelection();
    bool bBack = rSearchItem.GetBackward();
    bool bEverFound = false;

    for (SCCOL i=0; i < aCol.size(); ++i)
    {
        bool bFound = true;
        SCROW nRow = 0;
        SCROW nEndRow;
        while (bFound && nRow <= rDocument.MaxRow())
        {
            bFound = aCol[i].SearchStyleRange( nRow, nEndRow, pSearchStyle, bBack, bSelect, rMark );
            if (bFound)
            {
                if (nEndRow<nRow)
                {
                    SCROW nTemp = nRow;
                    nRow = nEndRow;
                    nEndRow = nTemp;
                }
                rMatchedRanges.Join(ScRange(i, nRow, nTab, i, nEndRow, nTab));
                nRow = nEndRow + 1;
                bEverFound = true;
            }
        }
    }

    return bEverFound;
}

bool ScTable::ReplaceAllStyle(
    const SvxSearchItem& rSearchItem, const ScMarkData& rMark, ScRangeList& rMatchedRanges,
    ScDocument* pUndoDoc)
{
    bool bRet = SearchAllStyle(rSearchItem, rMark, rMatchedRanges);
    if (bRet)
    {
        const ScStyleSheet* pReplaceStyle = static_cast<const ScStyleSheet*>(
                                        rDocument.GetStyleSheetPool()->Find(
                                        rSearchItem.GetReplaceString(), SfxStyleFamily::Para ));

        if (pReplaceStyle)
        {
            if (pUndoDoc)
                rDocument.CopyToDocument(0, 0 ,nTab, rDocument.MaxCol(),rDocument.MaxRow(),nTab,
                                          InsertDeleteFlags::ATTRIB, true, *pUndoDoc, &rMark);
            ApplySelectionStyle( *pReplaceStyle, rMark );
        }
        else
        {
            OSL_FAIL("pReplaceStyle==0");
        }
    }

    return bRet;
}

bool ScTable::SearchAndReplace(
    const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark,
    ScRangeList& rMatchedRanges, OUString& rUndoStr, ScDocument* pUndoDoc)
{
    SvxSearchCmd nCommand = rSearchItem.GetCommand();
    bool bFound = false;
    if ( ValidColRow(rCol, rRow) ||
         ((nCommand == SvxSearchCmd::FIND || nCommand == SvxSearchCmd::REPLACE) &&
           (((rCol == MAXCOLCOUNT || rCol == -1) && ValidRow(rRow)) ||
            ((rRow == GetDoc().GetSheetLimits().GetMaxRowCount() || rRow == -1) && ValidCol(rCol))
           )
         )
       )
    {
        bool bStyles = rSearchItem.GetPattern();
        if (bStyles)
        {
            if (nCommand == SvxSearchCmd::FIND)
                bFound = SearchStyle(rSearchItem, rCol, rRow, rMark);
            else if (nCommand == SvxSearchCmd::REPLACE)
                bFound = ReplaceStyle(rSearchItem, rCol, rRow, rMark, false);
            else if (nCommand == SvxSearchCmd::FIND_ALL)
                bFound = SearchAllStyle(rSearchItem, rMark, rMatchedRanges);
            else if (nCommand == SvxSearchCmd::REPLACE_ALL)
                bFound = ReplaceAllStyle(rSearchItem, rMark, rMatchedRanges, pUndoDoc);
        }
        else
        {
            //  SearchParam no longer needed - SearchOptions contains all settings
            i18nutil::SearchOptions2 aSearchOptions = rSearchItem.GetSearchOptions();
            aSearchOptions.Locale = *ScGlobal::GetLocale();

            if (aSearchOptions.searchString.isEmpty() || ( rSearchItem.GetRegExp() && aSearchOptions.searchString == "^$" ) )
            {
                // Search for empty cells.
                return SearchAndReplaceEmptyCells(rSearchItem, rCol, rRow, rMark, rMatchedRanges, rUndoStr, pUndoDoc);
            }

            //  reflect UseAsianOptions flag in SearchOptions
            //  (use only ignore case and width if asian options are disabled).
            //  This is also done in SvxSearchDialog CommandHdl, but not in API object.
            if ( !rSearchItem.IsUseAsianOptions() )
                aSearchOptions.transliterateFlags &=
                    TransliterationFlags::IGNORE_CASE |
                      TransliterationFlags::IGNORE_WIDTH;

            pSearchText.reset( new utl::TextSearch( aSearchOptions ) );

            if (nCommand == SvxSearchCmd::FIND)
                bFound = Search(rSearchItem, rCol, rRow, rMark, rUndoStr, pUndoDoc);
            else if (nCommand == SvxSearchCmd::FIND_ALL)
                bFound = SearchAll(rSearchItem, rMark, rMatchedRanges, rUndoStr, pUndoDoc);
            else if (nCommand == SvxSearchCmd::REPLACE)
                bFound = Replace(rSearchItem, rCol, rRow, rMark, rUndoStr, pUndoDoc);
            else if (nCommand == SvxSearchCmd::REPLACE_ALL)
                bFound = ReplaceAll(rSearchItem, rMark, rMatchedRanges, rUndoStr, pUndoDoc);

            pSearchText.reset();
        }
    }
    return bFound;
}

bool ScTable::SearchAndReplaceEmptyCells(
    const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark,
    ScRangeList& rMatchedRanges, OUString& rUndoStr, ScDocument* pUndoDoc)
{
    SCCOL nColStart, nColEnd;
    SCROW nRowStart, nRowEnd;
    GetFirstDataPos(nColStart, nRowStart);
    GetLastDataPos(nColEnd, nRowEnd);

    ScRangeList aRanges(ScRange(nColStart, nRowStart, nTab, nColEnd, nRowEnd, nTab));

    if (rSearchItem.GetSelection())
    {
        // current selection only.
        if (!rMark.IsMarked() && !rMark.IsMultiMarked())
            // There is no selection.  Bail out.
            return false;

        ScRangeList aMarkedRanges, aNewRanges;
        rMark.FillRangeListWithMarks(&aMarkedRanges, true);
        for ( size_t i = 0, n = aMarkedRanges.size(); i < n; ++i )
        {
            ScRange & rRange = aMarkedRanges[ i ];
            if (rRange.aStart.Col() > nColEnd || rRange.aStart.Row() > nRowEnd || rRange.aEnd.Col() < nColStart || rRange.aEnd.Row() < nRowStart)
                // This range is outside the data area.  Skip it.
                continue;

            // Shrink the range into data area only.
            if (rRange.aStart.Col() < nColStart)
                rRange.aStart.SetCol(nColStart);
            if (rRange.aStart.Row() < nRowStart)
                rRange.aStart.SetRow(nRowStart);

            if (rRange.aEnd.Col() > nColEnd)
                rRange.aEnd.SetCol(nColEnd);
            if (rRange.aEnd.Row() > nRowEnd)
                rRange.aEnd.SetRow(nRowEnd);

            aNewRanges.push_back(rRange);
        }
        aRanges = aNewRanges;
    }

    SvxSearchCmd nCommand = rSearchItem.GetCommand();
    if (nCommand == SvxSearchCmd::FIND || nCommand == SvxSearchCmd::REPLACE)
    {
        if (rSearchItem.GetBackward())
        {
            for ( size_t i = aRanges.size(); i > 0; --i )
            {
                const ScRange & rRange = aRanges[ i - 1 ];
                if (SearchRangeForEmptyCell(rRange, rSearchItem, rCol, rRow, rUndoStr))
                    return true;
            }
        }
        else
        {
            for ( size_t i = 0, nListSize = aRanges.size(); i < nListSize; ++i )
            {
                const ScRange & rRange = aRanges[ i ];
                if (SearchRangeForEmptyCell(rRange, rSearchItem, rCol, rRow, rUndoStr))
                    return true;
            }
        }
    }
    else if (nCommand == SvxSearchCmd::FIND_ALL || nCommand == SvxSearchCmd::REPLACE_ALL)
    {
        bool bFound = false;
        for ( size_t i = 0, nListSize = aRanges.size(); i < nListSize; ++i )
        {
            ScRange const & rRange = aRanges[ i ];
            bFound |= SearchRangeForAllEmptyCells(rRange, rSearchItem, rMatchedRanges, rUndoStr, pUndoDoc);
        }
        return bFound;
    }
    return false;
}

namespace {

bool lcl_maybeReplaceCellString(
    ScColumn& rColObj, SCCOL& rCol, SCROW& rRow, OUString& rUndoStr, SCCOL nCol, SCROW nRow, const SvxSearchItem& rSearchItem)
{
    ScRefCellValue aCell = rColObj.GetCellValue(nRow);
    if (aCell.isEmpty())
    {
        // empty cell found.
        rCol = nCol;
        rRow = nRow;
        if (rSearchItem.GetCommand() == SvxSearchCmd::REPLACE &&
            !rSearchItem.GetReplaceString().isEmpty())
        {
            rColObj.SetRawString(nRow, rSearchItem.GetReplaceString());
            rUndoStr.clear();
        }
        return true;
    }
    return false;
}

}

bool ScTable::SearchRangeForEmptyCell(
    const ScRange& rRange, const SvxSearchItem& rSearchItem,
    SCCOL& rCol, SCROW& rRow, OUString& rUndoStr)
{
    SvxSearchCmd nCmd = rSearchItem.GetCommand();
    bool bSkipFiltered = rSearchItem.IsSearchFiltered();
    if (rSearchItem.GetBackward())
    {
        // backward search
        if (rSearchItem.GetRowDirection())
        {
            // row direction.
            SCROW nLastNonFilteredRow = rDocument.MaxRow() + 1;
            SCROW nBeginRow = std::min(rRange.aEnd.Row(), rRow);
            for (SCROW nRow = nBeginRow; nRow >= rRange.aStart.Row(); --nRow)
            {
                if (bSkipFiltered)
                    SkipFilteredRows(nRow, nLastNonFilteredRow, false);
                if (nRow < rRange.aStart.Row())
                    break;

                SCCOL nBeginCol = rRange.aEnd.Col();
                if (nRow == rRow && nBeginCol >= rCol)
                    // always start from one cell before the cursor.
                    nBeginCol = rCol - (nCmd == SvxSearchCmd::FIND ? 1 : 0);

                for (SCCOL nCol = nBeginCol; nCol >= rRange.aStart.Col(); --nCol)
                {
                    if (lcl_maybeReplaceCellString(aCol[nCol], rCol, rRow, rUndoStr, nCol, nRow, rSearchItem))
                        return true;
                }
            }
        }
        else
        {
            // column direction.
            SCCOL nBeginCol = std::min(rRange.aEnd.Col(), rCol);
            for (SCCOL nCol = nBeginCol; nCol >= rRange.aStart.Col(); --nCol)
            {
                SCROW nLastNonFilteredRow = rDocument.MaxRow() + 1;
                SCROW nBeginRow = rRange.aEnd.Row();
                if (nCol == rCol && nBeginRow >= rRow)
                    // always start from one cell before the cursor.
                    nBeginRow = rRow - (nCmd == SvxSearchCmd::FIND ? 1 : 0);
                for (SCROW nRow = nBeginRow; nRow >= rRange.aStart.Row(); --nRow)
                {
                    if (bSkipFiltered)
                        SkipFilteredRows(nRow, nLastNonFilteredRow, false);
                    if (nRow < rRange.aStart.Row())
                        break;

                    if (lcl_maybeReplaceCellString(aCol[nCol], rCol, rRow, rUndoStr, nCol, nRow, rSearchItem))
                        return true;
                }
            }
        }
    }
    else
    {
        // forward search
        if (rSearchItem.GetRowDirection())
        {
            // row direction.
            SCROW nLastNonFilteredRow = -1;
            SCROW nBeginRow = rRange.aStart.Row() < rRow ? rRow : rRange.aStart.Row();
            for (SCROW nRow = nBeginRow; nRow <= rRange.aEnd.Row(); ++nRow)
            {
                if (bSkipFiltered)
                    SkipFilteredRows(nRow, nLastNonFilteredRow, true);
                if (nRow > rRange.aEnd.Row())
                    break;

                SCCOL nBeginCol = rRange.aStart.Col();
                if (nRow == rRow && nBeginCol <= rCol)
                    // always start from one cell past the cursor.
                    nBeginCol = rCol + (nCmd == SvxSearchCmd::FIND ? 1 : 0);
                for (SCCOL nCol = nBeginCol; nCol <= rRange.aEnd.Col(); ++nCol)
                {
                    if (lcl_maybeReplaceCellString(aCol[nCol], rCol, rRow, rUndoStr, nCol, nRow, rSearchItem))
                        return true;
                }
            }
        }
        else
        {
            // column direction.
            SCCOL nBeginCol = rRange.aStart.Col() < rCol ? rCol : rRange.aStart.Col();
            for (SCCOL nCol = nBeginCol; nCol <= rRange.aEnd.Col(); ++nCol)
            {
                SCROW nLastNonFilteredRow = -1;
                SCROW nBeginRow = rRange.aStart.Row();
                if (nCol == rCol && nBeginRow <= rRow)
                    // always start from one cell past the cursor.
                    nBeginRow = rRow + (nCmd == SvxSearchCmd::FIND ? 1 : 0);
                for (SCROW nRow = nBeginRow; nRow <= rRange.aEnd.Row(); ++nRow)
                {
                    if (bSkipFiltered)
                        SkipFilteredRows(nRow, nLastNonFilteredRow, true);
                    if (nRow > rRange.aEnd.Row())
                        break;

                    if (lcl_maybeReplaceCellString(aCol[nCol], rCol, rRow, rUndoStr, nCol, nRow, rSearchItem))
                        return true;
                }
            }
        }
    }
    return false;
}

bool ScTable::SearchRangeForAllEmptyCells(
    const ScRange& rRange, const SvxSearchItem& rSearchItem,
    ScRangeList& rMatchedRanges, OUString& rUndoStr, ScDocument* pUndoDoc)
{
    bool bFound = false;
    bool bReplace = (rSearchItem.GetCommand() == SvxSearchCmd::REPLACE_ALL) &&
                    !rSearchItem.GetReplaceString().isEmpty();
    bool bSkipFiltered = rSearchItem.IsSearchFiltered();

    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        SCROW nLastNonFilteredRow = -1;
        if (aCol[nCol].IsEmptyData())
        {
            // The entire column is empty.
            const SCROW nEndRow = rRange.aEnd.Row();
            for (SCROW nRow = rRange.aStart.Row(); nRow <= nEndRow; ++nRow)
            {
                SCROW nLastRow;
                const bool bFiltered = RowFiltered(nRow, nullptr, &nLastRow);
                if (nLastRow > nEndRow)
                    nLastRow = nEndRow;
                if (!bFiltered)
                {
                    rMatchedRanges.Join(ScRange(nCol, nRow, nTab, nCol, nLastRow, nTab));
                    if (bReplace)
                    {
                        const OUString& rNewStr = rSearchItem.GetReplaceString();
                        for (SCROW i = nRow; i <= nLastRow; ++i)
                        {
                            aCol[nCol].SetRawString(i, rNewStr);
                            if (pUndoDoc)
                            {
                                // TODO: I'm using a string cell with empty content to
                                // trigger deletion of cell instance on undo.  Maybe I
                                // should create a new cell type for this?
                                pUndoDoc->SetString(ScAddress(nCol, i, nTab), EMPTY_OUSTRING);
                            }
                        }
                        rUndoStr.clear();
                    }
                }

                nRow = nLastRow; // move to the last filtered row.
            }
            bFound = true;
            continue;
        }

        for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
        {
            if (bSkipFiltered)
                SkipFilteredRows(nRow, nLastNonFilteredRow, true);
            if (nRow > rRange.aEnd.Row())
                break;

            ScRefCellValue aCell = aCol[nCol].GetCellValue(nRow);
            if (aCell.isEmpty())
            {
                // empty cell found
                rMatchedRanges.Join(ScRange(nCol, nRow, nTab));
                bFound = true;

                if (bReplace)
                {
                    aCol[nCol].SetRawString(nRow, rSearchItem.GetReplaceString());
                    if (pUndoDoc)
                    {
                        // TODO: I'm using a string cell with empty content to
                        // trigger deletion of cell instance on undo.  Maybe I
                        // should create a new cell type for this?
                        pUndoDoc->SetString(ScAddress(nCol, nRow, nTab), EMPTY_OUSTRING);
                    }
                }
            }
        }
    }
    return bFound;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
