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

#include <com/sun/star/i18n/TransliterationModules.hpp>

#include <unotools/textsearch.hxx>
#include <svl/srchitem.hxx>
#include <editeng/editobj.hxx>

#include "table.hxx"
#include "collect.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "stlpool.hxx"
#include "markdata.hxx"
#include "editutil.hxx"
#include "detfunc.hxx"
#include "postit.hxx"

//--------------------------------------------------------------------------


using ::com::sun::star::util::SearchOptions;

BOOL lcl_GetTextWithBreaks( const ScEditCell& rCell, ScDocument* pDoc, String& rVal )
{
    //  TRUE = more than 1 paragraph

    const EditTextObject* pData = NULL;
    rCell.GetData( pData );
    EditEngine& rEngine = pDoc->GetEditEngine();
    rEngine.SetText( *pData );
    rVal = rEngine.GetText( LINEEND_LF );
    return ( rEngine.GetParagraphCount() > 1 );
}

BOOL ScTable::SearchCell(const SvxSearchItem& rSearchItem, SCCOL nCol, SCROW nRow,
                            const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc)
{
    BOOL    bFound = FALSE;
    BOOL    bDoSearch = TRUE;
    BOOL    bDoBack = rSearchItem.GetBackward();

    String  aString;
    ScBaseCell* pCell;
    if (rSearchItem.GetSelection())
        bDoSearch = rMark.IsCellMarked(nCol, nRow);
    if ( bDoSearch && ((pCell = aCol[nCol].GetCell( nRow )) != NULL) )
    {
        BOOL bMultiLine = FALSE;
        CellType eCellType = pCell->GetCellType();
        switch (rSearchItem.GetCellType())
        {
            case SVX_SEARCHIN_FORMULA:
            {
                if ( eCellType == CELLTYPE_FORMULA )
                    ((ScFormulaCell*)pCell)->GetFormula( aString,
                       formula::FormulaGrammar::GRAM_NATIVE_UI);
                else if ( eCellType == CELLTYPE_EDIT )
                    bMultiLine = lcl_GetTextWithBreaks(
                        *(const ScEditCell*)pCell, pDocument, aString );
                else
                    aCol[nCol].GetInputString( nRow, aString );
            }
            break;
            case SVX_SEARCHIN_VALUE:
                if ( eCellType == CELLTYPE_EDIT )
                    bMultiLine = lcl_GetTextWithBreaks(
                        *(const ScEditCell*)pCell, pDocument, aString );
                else
                    aCol[nCol].GetInputString( nRow, aString );
                break;
            case SVX_SEARCHIN_NOTE:
                {
                    if(const ScPostIt* pNote = pCell->GetNote())
                    {
                        aString = pNote->GetText();
                        bMultiLine = pNote->HasMultiLineText();
                    }
                }
                break;
            default:
                break;
        }
        xub_StrLen nStart = 0;
        xub_StrLen nEnd = aString.Len();
        ::com::sun::star::util::SearchResult aSearchResult;
        if (pSearchText)
        {
            if ( bDoBack )
            {
                xub_StrLen nTemp=nStart; nStart=nEnd; nEnd=nTemp;
                bFound = (BOOL)(pSearchText->SearchBkwrd(aString, &nStart, &nEnd, &aSearchResult));
                // change results to definition before 614:
                --nEnd;
            }
            else
            {
                bFound = (BOOL)(pSearchText->SearchFrwrd(aString, &nStart, &nEnd, &aSearchResult));
                // change results to definition before 614:
                --nEnd;
            }

            if (bFound && rSearchItem.GetWordOnly())
                bFound = (nStart == 0 && nEnd == aString.Len() - 1);
        }
        else
        {
            OSL_FAIL("pSearchText == NULL");
            return bFound;
        }

        BYTE cMatrixFlag = MM_NONE;
        if ( bFound &&
            ( (rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE)
            ||(rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL) ) &&
                // #60558# Matrix nicht zerreissen, nur Matrixformel ersetzen
                !( (eCellType == CELLTYPE_FORMULA &&
                ((cMatrixFlag = ((ScFormulaCell*)pCell)->GetMatrixFlag()) == MM_REFERENCE))
                // kein UndoDoc => Matrix nicht wiederherstellbar => nicht ersetzen
                || (cMatrixFlag != MM_NONE && !pUndoDoc) )
            )
        {
            if ( cMatrixFlag == MM_NONE && rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE )
                rUndoStr = aString;
            else if (pUndoDoc)
            {
                ScAddress aAdr( nCol, nRow, nTab );
                ScBaseCell* pUndoCell = pCell->CloneWithoutNote( *pUndoDoc );
                pUndoDoc->PutCell( aAdr, pUndoCell);
            }
            BOOL bRepeat = !rSearchItem.GetWordOnly();
            do
            {
                //  wenn der gefundene Text leer ist, nicht weitersuchen,
                //  sonst wuerde man nie mehr aufhoeren (#35410#)
                if ( nEnd < nStart || nEnd == STRING_MAXLEN )
                    bRepeat = FALSE;

                String sReplStr = rSearchItem.GetReplaceString();
                if (rSearchItem.GetRegExp())
                {
                    String sFndStr = aString.Copy(nStart, nEnd-nStart+1);
                    pSearchText->ReplaceBackReferences( sReplStr, aString, aSearchResult );
                    aString.Erase(nStart, nEnd-nStart+1);
                    aString.Insert(sReplStr, nStart);
                }
                else
                {
                    aString.Erase(nStart, nEnd - nStart + 1);
                    aString.Insert(rSearchItem.GetReplaceString(), nStart);
                }

                        //  Indizes anpassen
                if (bDoBack)
                {
                    nEnd = nStart;
                    nStart = 0;
                }
                else
                {
                    nStart = sal::static_int_cast<xub_StrLen>( nStart + sReplStr.Len() );
                    nEnd = aString.Len();
                }

                        //  weitersuchen ?
                if (bRepeat)
                {
                    if ( rSearchItem.GetCommand() != SVX_SEARCHCMD_REPLACE_ALL || nStart >= nEnd )
                        bRepeat = FALSE;
                    else if (bDoBack)
                    {
                        xub_StrLen nTemp=nStart; nStart=nEnd; nEnd=nTemp;
                        bRepeat = ((BOOL)(pSearchText->SearchBkwrd(aString, &nStart, &nEnd)));
                        // change results to definition before 614:
                        --nEnd;
                    }
                    else
                    {
                        bRepeat = ((BOOL)(pSearchText->SearchFrwrd(aString, &nStart, &nEnd)));
                        // change results to definition before 614:
                        --nEnd;
                    }
                }
            }
            while (bRepeat);
            if (rSearchItem.GetCellType() == SVX_SEARCHIN_NOTE)
            {
                // NB: rich text format is lost.
                // This is also true of Cells.
                if( ScPostIt* pNote = pCell->GetNote() )
                    pNote->SetText( ScAddress( nCol, nRow, nTab ), aString );
            }
            else if ( cMatrixFlag != MM_NONE )
            {   // #60558# Matrix nicht zerreissen
                if ( aString.Len() > 2 )
                {   // {} raus, erst hier damit auch "{=" durch "{=..." ersetzt werden kann
                    if ( aString.GetChar( aString.Len()-1 ) == '}' )
                        aString.Erase( aString.Len()-1, 1 );
                    if ( aString.GetChar(0) == '{' )
                        aString.Erase( 0, 1 );
                }
                ScAddress aAdr( nCol, nRow, nTab );
                ScFormulaCell* pFCell = new ScFormulaCell( pDocument, aAdr,
                    aString,formula::FormulaGrammar::GRAM_NATIVE_UI, cMatrixFlag );
                SCCOL nMatCols;
                SCROW nMatRows;
                ((ScFormulaCell*)pCell)->GetMatColsRows( nMatCols, nMatRows );
                pFCell->SetMatColsRows( nMatCols, nMatRows );
                aCol[nCol].Insert( nRow, pFCell );
            }
            else if ( bMultiLine && aString.Search('\n') != STRING_NOTFOUND )
                PutCell( nCol, nRow, new ScEditCell( aString, pDocument ) );
            else
                aCol[nCol].SetString(nRow, nTab, aString);
            // pCell is invalid now (deleted)
        }
    }
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
        if (RowFiltered(rRow, NULL, &nLastRow))
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
        if (RowFiltered(rRow, &nFirstRow, NULL))
            // move to the first non-filtered row.
            rRow = nFirstRow - 1;
        else
            // record the last non-filtered row to avoid checking
            // the filtered state for each and every row.
            rLastNonFilteredRow = nFirstRow;
    }
}

BOOL ScTable::Search(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc)
{
    BOOL bFound = FALSE;
    BOOL bAll =  (rSearchItem.GetCommand() == SVX_SEARCHCMD_FIND_ALL)
               ||(rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL);
    SCCOL nCol = rCol;
    SCROW nRow = rRow;
    SCCOL nLastCol;
    SCROW nLastRow;
    GetLastDataPos(nLastCol, nLastRow);
    bool bSkipFiltered = !rSearchItem.IsSearchFiltered();
    if (!bAll && rSearchItem.GetBackward())
    {
        SCROW nLastNonFilteredRow = MAXROW + 1;
        nCol = Min(nCol, (SCCOL)(nLastCol + 1));
        nRow = Min(nRow, (SCROW)(nLastRow + 1));
        if (rSearchItem.GetRowDirection())
        {
            nCol--;
            while (!bFound && ((SCsROW)nRow >= 0))
            {
                if (bSkipFiltered)
                    SkipFilteredRows(nRow, nLastNonFilteredRow, false);

                while (!bFound && ((SCsCOL)nCol >= 0))
                {
                    bFound = SearchCell(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                        BOOL bIsEmpty;
                        do
                        {
                            nCol--;
                            if ((SCsCOL)nCol >= 0)
                                bIsEmpty = aCol[nCol].IsEmptyData();
                            else
                                bIsEmpty = TRUE;
                        }
                        while (((SCsCOL)nCol >= 0) && bIsEmpty);
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
            while (!bFound && ((SCsCOL)nCol >= 0))
            {
                while (!bFound && ((SCsROW)nRow >= 0))
                {
                    if (bSkipFiltered)
                        SkipFilteredRows(nRow, nLastNonFilteredRow, false);

                    bFound = SearchCell(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                         if (!aCol[nCol].GetPrevDataPos(nRow))
                            nRow = -1;
                    }
                }
                if (!bFound)
                {
                    // Not found in this column.  Move to the next column.
                    BOOL bIsEmpty;
                    nRow = nLastRow;
                    nLastNonFilteredRow = MAXROW + 1;
                    do
                    {
                        nCol--;
                        if ((SCsCOL)nCol >= 0)
                            bIsEmpty = aCol[nCol].IsEmptyData();
                        else
                            bIsEmpty = TRUE;
                    }
                    while (((SCsCOL)nCol >= 0) && bIsEmpty);
                }
            }
        }
    }
    else
    {
        SCROW nLastNonFilteredRow = -1;
        if (!bAll && rSearchItem.GetRowDirection())
        {
            nCol++;
            while (!bFound && (nRow <= nLastRow))
            {
                if (bSkipFiltered)
                    SkipFilteredRows(nRow, nLastNonFilteredRow, true);

                while (!bFound && (nCol <= nLastCol))
                {
                    bFound = SearchCell(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                        nCol++;
                        while ((nCol <= nLastCol) && aCol[nCol].IsEmptyData()) nCol++;
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

                    bFound = SearchCell(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                         if (!aCol[nCol].GetNextDataPos(nRow))
                            nRow = MAXROW + 1;
                    }
                }
                if (!bFound)
                {
                    // Not found in this column.  Move to the next column.
                    nRow = 0;
                    nLastNonFilteredRow = -1;
                    nCol++;
                    while ((nCol <= nLastCol) && aCol[nCol].IsEmptyData()) nCol++;
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

BOOL ScTable::SearchAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                        String& rUndoStr, ScDocument* pUndoDoc)
{
    BOOL bFound = TRUE;
    SCCOL nCol = 0;
    SCROW nRow = -1;

    ScMarkData aNewMark( rMark );   // Tabellen-Markierungen kopieren
    aNewMark.ResetMark();
    do
    {
        bFound = Search(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
        if (bFound)
            aNewMark.SetMultiMarkArea( ScRange( nCol, nRow, nTab ) );
    }
    while (bFound);

    rMark = aNewMark;       //  Markierung kopieren
                            //! pro Tabelle

    return (aNewMark.IsMultiMarked());
}

BOOL ScTable::Replace(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc)
{
    BOOL bFound = FALSE;
    SCCOL nCol = rCol;
    SCROW nRow = rRow;
    if (rSearchItem.GetBackward())
    {
        if (rSearchItem.GetRowDirection())
            nCol += 1;
        else
            nRow += 1;
    }
    else
    {
        if (rSearchItem.GetRowDirection())
            nCol -= 1;
        else
            nRow -= 1;
    }
    bFound = Search(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
    if (bFound)
    {
        rCol = nCol;
        rRow = nRow;
    }
    return bFound;
}

BOOL ScTable::ReplaceAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                            String& rUndoStr, ScDocument* pUndoDoc)
{
    BOOL bOldDouble = ScColumn::bDoubleAlloc;       // sollte immer FALSE sein?
    DBG_ASSERT(!bOldDouble,"bDoubleAlloc ???");
    ScColumn::bDoubleAlloc = TRUE;                  // fuer Undo-Doc

    BOOL bFound = TRUE;
    SCCOL nCol = 0;
    SCROW nRow = -1;

    ScMarkData aNewMark( rMark );   // Tabellen-Markierungen kopieren
    aNewMark.ResetMark();
    do
    {
        bFound = Search(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
        if (bFound)
            aNewMark.SetMultiMarkArea( ScRange( nCol, nRow, nTab ) );
    }
    while (bFound);

    ScColumn::bDoubleAlloc = bOldDouble;

    rMark = aNewMark;       //  Markierung kopieren
                            //! pro Tabelle

    return (aNewMark.IsMultiMarked());
}

BOOL ScTable::SearchStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                            ScMarkData& rMark)
{
    const ScStyleSheet* pSearchStyle = (const ScStyleSheet*)
                                        pDocument->GetStyleSheetPool()->Find(
                                        rSearchItem.GetSearchString(), SFX_STYLE_FAMILY_PARA );

    SCsCOL nCol = rCol;
    SCsROW nRow = rRow;
    BOOL bFound = FALSE;

    BOOL bSelect = rSearchItem.GetSelection();
    BOOL bRows = rSearchItem.GetRowDirection();
    BOOL bBack = rSearchItem.GetBackward();
    short nAdd = bBack ? -1 : 1;

    if (bRows)                                      // zeilenweise
    {
        nRow += nAdd;
        do
        {
            SCsROW nNextRow = aCol[nCol].SearchStyle( nRow, pSearchStyle, bBack, bSelect, rMark );
            if (!ValidRow(nNextRow))
            {
                nRow = bBack ? MAXROW : 0;
                nCol = sal::static_int_cast<SCsCOL>( nCol + nAdd );
            }
            else
            {
                nRow = nNextRow;
                bFound = TRUE;
            }
        }
        while (!bFound && ValidCol(nCol));
    }
    else                                            // spaltenweise
    {
        SCsROW nNextRows[MAXCOLCOUNT];
        SCsCOL i;
        for (i=0; i<=MAXCOL; i++)
        {
            SCsROW nSRow = nRow;
            if (bBack)  { if (i>=nCol) --nSRow; }
            else        { if (i<=nCol) ++nSRow; }
            nNextRows[i] = aCol[i].SearchStyle( nSRow, pSearchStyle, bBack, bSelect, rMark );
        }
        if (bBack)                          // rueckwaerts
        {
            nRow = -1;
            for (i=MAXCOL; i>=0; i--)
                if (nNextRows[i]>nRow)
                {
                    nCol = i;
                    nRow = nNextRows[i];
                    bFound = TRUE;
                }
        }
        else                                // vorwaerts
        {
            nRow = MAXROW+1;
            for (i=0; i<=MAXCOL; i++)
                if (nNextRows[i]<nRow)
                {
                    nCol = i;
                    nRow = nNextRows[i];
                    bFound = TRUE;
                }
        }
    }

    if (bFound)
    {
        rCol = (SCCOL) nCol;
        rRow = (SCROW) nRow;
    }
    return bFound;
}

//!     einzelnes Pattern fuer Undo zurueckgeben

BOOL ScTable::ReplaceStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                           ScMarkData& rMark, BOOL bIsUndo)
{
    BOOL bRet;
    if (bIsUndo)
        bRet = TRUE;
    else
        bRet = SearchStyle(rSearchItem, rCol, rRow, rMark);
    if (bRet)
    {
        const ScStyleSheet* pReplaceStyle = (const ScStyleSheet*)
                                        pDocument->GetStyleSheetPool()->Find(
                                        rSearchItem.GetReplaceString(), SFX_STYLE_FAMILY_PARA );

        if (pReplaceStyle)
            ApplyStyle( rCol, rRow, *pReplaceStyle );
        else
        {
            OSL_FAIL("pReplaceStyle==0");
        }
    }

    return bRet;
}

BOOL ScTable::SearchAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark)
{
    const ScStyleSheet* pSearchStyle = (const ScStyleSheet*)
                                        pDocument->GetStyleSheetPool()->Find(
                                        rSearchItem.GetSearchString(), SFX_STYLE_FAMILY_PARA );
    BOOL bSelect = rSearchItem.GetSelection();
    BOOL bBack = rSearchItem.GetBackward();

    ScMarkData aNewMark( rMark );   // Tabellen-Markierungen kopieren
    aNewMark.ResetMark();
    for (SCCOL i=0; i<=MAXCOL; i++)
    {
        BOOL bFound = TRUE;
        SCsROW nRow = 0;
        SCsROW nEndRow;
        while (bFound && nRow <= MAXROW)
        {
            bFound = aCol[i].SearchStyleRange( nRow, nEndRow, pSearchStyle, bBack, bSelect, rMark );
            if (bFound)
            {
                if (nEndRow<nRow)
                {
                    SCsROW nTemp = nRow;
                    nRow = nEndRow;
                    nEndRow = nTemp;
                }
                aNewMark.SetMultiMarkArea( ScRange( i,nRow,nTab, i,nEndRow,nTab ) );
                nRow = nEndRow + 1;
            }
        }
    }

    rMark = aNewMark;       //  Markierung kopieren
                            //! pro Tabelle

    return (aNewMark.IsMultiMarked());
}

BOOL ScTable::ReplaceAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                                ScDocument* pUndoDoc)
{
    BOOL bRet = SearchAllStyle(rSearchItem, rMark);
    if (bRet)
    {
        const ScStyleSheet* pReplaceStyle = (const ScStyleSheet*)
                                        pDocument->GetStyleSheetPool()->Find(
                                        rSearchItem.GetReplaceString(), SFX_STYLE_FAMILY_PARA );

        if (pReplaceStyle)
        {
            if (pUndoDoc)
                pDocument->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                            IDF_ATTRIB, TRUE, pUndoDoc, &rMark );
            ApplySelectionStyle( *pReplaceStyle, rMark );
        }
        else
        {
            OSL_FAIL("pReplaceStyle==0");
        }
    }

    return bRet;
}

BOOL ScTable::SearchAndReplace(const SvxSearchItem& rSearchItem,
                                SCCOL& rCol, SCROW& rRow, ScMarkData& rMark,
                                String& rUndoStr, ScDocument* pUndoDoc)
{
    USHORT nCommand = rSearchItem.GetCommand();
    BOOL bFound = FALSE;
    if ( ValidColRow(rCol, rRow) ||
         ((nCommand == SVX_SEARCHCMD_FIND || nCommand == SVX_SEARCHCMD_REPLACE) &&
           (((rCol == MAXCOLCOUNT || rCol == -1) && VALIDROW(rRow)) ||
            ((rRow == MAXROWCOUNT || rRow == -1) && VALIDCOL(rCol))
           )
         )
       )
    {
        BOOL bStyles = rSearchItem.GetPattern();
        if (bStyles)
        {
            if (nCommand == SVX_SEARCHCMD_FIND)
                bFound = SearchStyle(rSearchItem, rCol, rRow, rMark);
            else if (nCommand == SVX_SEARCHCMD_REPLACE)
                bFound = ReplaceStyle(rSearchItem, rCol, rRow, rMark, FALSE);
            else if (nCommand == SVX_SEARCHCMD_FIND_ALL)
                bFound = SearchAllStyle(rSearchItem, rMark);
            else if (nCommand == SVX_SEARCHCMD_REPLACE_ALL)
                bFound = ReplaceAllStyle(rSearchItem, rMark, pUndoDoc);
        }
        else
        {
            //  SearchParam no longer needed - SearchOptions contains all settings
            com::sun::star::util::SearchOptions aSearchOptions = rSearchItem.GetSearchOptions();
            aSearchOptions.Locale = *ScGlobal::GetLocale();

            if (!aSearchOptions.searchString.getLength())
            {
                // Search for empty cells.
                return SearchAndReplaceEmptyCells(rSearchItem, rCol, rRow, rMark, rUndoStr, pUndoDoc);
            }

            //  #107259# reflect UseAsianOptions flag in SearchOptions
            //  (use only ignore case and width if asian options are disabled).
            //  This is also done in SvxSearchDialog CommandHdl, but not in API object.
            if ( !rSearchItem.IsUseAsianOptions() )
                aSearchOptions.transliterateFlags &=
                    ( com::sun::star::i18n::TransliterationModules_IGNORE_CASE |
                      com::sun::star::i18n::TransliterationModules_IGNORE_WIDTH );

            pSearchText = new utl::TextSearch( aSearchOptions );

            if (nCommand == SVX_SEARCHCMD_FIND)
                bFound = Search(rSearchItem, rCol, rRow, rMark, rUndoStr, pUndoDoc);
            else if (nCommand == SVX_SEARCHCMD_FIND_ALL)
                bFound = SearchAll(rSearchItem, rMark, rUndoStr, pUndoDoc);
            else if (nCommand == SVX_SEARCHCMD_REPLACE)
                bFound = Replace(rSearchItem, rCol, rRow, rMark, rUndoStr, pUndoDoc);
            else if (nCommand == SVX_SEARCHCMD_REPLACE_ALL)
                bFound = ReplaceAll(rSearchItem, rMark, rUndoStr, pUndoDoc);

            delete pSearchText;
            pSearchText = NULL;
        }
    }
    return bFound;
}

bool ScTable::SearchAndReplaceEmptyCells(
    const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow, ScMarkData& rMark,
    String& rUndoStr, ScDocument* pUndoDoc)
{
    SCCOL nColStart, nColEnd;
    SCROW nRowStart, nRowEnd;
    GetFirstDataPos(nColStart, nRowStart);
    GetLastDataPos(nColEnd, nRowEnd);

    ScRangeList aRanges;
    aRanges.Append(ScRange(nColStart, nRowStart, nTab, nColEnd, nRowEnd, nTab));

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
            ScRange* p = aMarkedRanges[ i ];
            if (p->aStart.Col() > nColEnd || p->aStart.Row() > nRowEnd)
                // This range is outside the data area.  Skip it.
                continue;

            // Shrink the range into data area only.
            if (p->aStart.Col() < nColStart)
                p->aStart.SetCol(rCol);
            if (p->aStart.Row() < nRowStart)
                p->aStart.SetRow(rRow);

            if (p->aEnd.Col() > nColEnd)
                p->aEnd.SetCol(nColEnd);
            if (p->aEnd.Row() > nRowEnd)
                p->aEnd.SetRow(nRowEnd);

            aNewRanges.Append(*p);
        }
        aRanges = aNewRanges;
    }

    sal_uInt16 nCommand = rSearchItem.GetCommand();
    if (nCommand == SVX_SEARCHCMD_FIND || nCommand == SVX_SEARCHCMD_REPLACE)
    {
        if (rSearchItem.GetBackward())
        {
            for ( size_t i = aRanges.size(); i > 0; --i )
            {
                ScRange* p = aRanges[ i - 1 ];
                if (SearchRangeForEmptyCell( *p, rSearchItem, rCol, rRow, rUndoStr, pUndoDoc))
                    return true;
            }
        }
        else
        {
            for ( size_t i = 0, nListSize = aRanges.size(); i < nListSize; ++i )
            {
                ScRange* p = aRanges[ i ];
                if (SearchRangeForEmptyCell( *p, rSearchItem, rCol, rRow, rUndoStr, pUndoDoc ))
                    return true;
            }
        }
    }
    else if (nCommand == SVX_SEARCHCMD_FIND_ALL || nCommand == SVX_SEARCHCMD_REPLACE_ALL)
    {
        bool bFound = false;
        ScMarkData aNewMark(rMark);
        aNewMark.ResetMark();
        for ( size_t i = 0, nListSize = aRanges.size(); i < nListSize; ++i )
        {
            ScRange* p = aRanges[ i ];
            bFound |= SearchRangeForAllEmptyCells(*p, rSearchItem, aNewMark, rUndoStr, pUndoDoc);
        }
        rMark = aNewMark;
        return bFound;
    }
    return false;
}

namespace {

bool lcl_maybeReplaceCellString(
    ScColumn& rColObj, SCCOL& rCol, SCROW& rRow, String& rUndoStr, SCCOL nCol, SCROW nRow, const SvxSearchItem& rSearchItem)
{
    ScBaseCell* pCell = rColObj.GetCell(nRow);
    if (!pCell || pCell->GetCellType() == CELLTYPE_NOTE)
    {
        // empty cell found.
        rCol = nCol;
        rRow = nRow;
        if (rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE &&
            rSearchItem.GetReplaceString().Len())
        {
            rColObj.Insert(nRow, new ScStringCell(rSearchItem.GetReplaceString()));
            rUndoStr = String();
        }
        return true;
    }
    return false;
}

}

bool ScTable::SearchRangeForEmptyCell(
    const ScRange& rRange, const SvxSearchItem& rSearchItem,
    SCCOL& rCol, SCROW& rRow, String& rUndoStr, ScDocument* /*pUndoDoc*/)
{
    sal_uInt16 nCmd = rSearchItem.GetCommand();
    bool bSkipFiltered = rSearchItem.IsSearchFiltered();
    if (rSearchItem.GetBackward())
    {
        // backward search
        if (rSearchItem.GetRowDirection())
        {
            // row direction.
            SCROW nLastNonFilteredRow = MAXROW + 1;
            SCROW nBeginRow = rRange.aEnd.Row() > rRow ? rRow : rRange.aEnd.Row();
            for (SCROW nRow = nBeginRow; nRow >= rRange.aStart.Row(); --nRow)
            {
                if (bSkipFiltered)
                    SkipFilteredRows(nRow, nLastNonFilteredRow, false);
                if (nRow < rRange.aStart.Row())
                    break;

                SCCOL nBeginCol = rRange.aEnd.Col();
                if (nRow == rRow && nBeginCol >= rCol)
                    // always start from one cell before the cursor.
                    nBeginCol = rCol - (nCmd == SVX_SEARCHCMD_FIND ? 1 : 0);

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
            SCCOL nBeginCol = rRange.aEnd.Col() > rCol ? rCol : rRange.aEnd.Col();
            for (SCCOL nCol = nBeginCol; nCol >= rRange.aStart.Col(); --nCol)
            {
                SCROW nLastNonFilteredRow = MAXROW + 1;
                SCROW nBeginRow = rRange.aEnd.Row();
                if (nCol == rCol && nBeginRow >= rRow)
                    // always start from one cell before the cursor.
                    nBeginRow = rRow - (nCmd == SVX_SEARCHCMD_FIND ? 1 : 0);
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
                    nBeginCol = rCol + (nCmd == SVX_SEARCHCMD_FIND ? 1 : 0);
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
                    nBeginRow = rRow + (nCmd == SVX_SEARCHCMD_FIND ? 1 : 0);
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
    const ScRange& rRange, const SvxSearchItem& rSearchItem, ScMarkData& rMark,
    String& rUndoStr, ScDocument* pUndoDoc)
{
    bool bFound = false;
    bool bReplace = (rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL) &&
                    (rSearchItem.GetReplaceString().Len() > 0);
    bool bSkipFiltered = rSearchItem.IsSearchFiltered();

    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        SCROW nLastNonFilteredRow = -1;
        if (aCol[nCol].IsEmptyData())
        {
            // The entire column is empty.
            for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
            {
                SCROW nLastRow;
                if (!RowFiltered(nRow, NULL, &nLastRow))
                {
                    rMark.SetMultiMarkArea(ScRange(nCol, nRow, nTab, nCol, nLastRow, nTab));
                    if (bReplace)
                    {
                        const String& rNewStr = rSearchItem.GetReplaceString();
                        for (SCROW i = nRow; i <= nLastRow; ++i)
                        {
                            aCol[nCol].Insert(i, new ScStringCell(rNewStr));
                            if (pUndoDoc)
                                // TODO: I'm using a string cell with empty content to
                                // trigger deletion of cell instance on undo.  Maybe I
                                // should create a new cell type for this?
                                pUndoDoc->PutCell(nCol, i, nTab, new ScStringCell(String()));
                        }
                        rUndoStr = String();
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

            ScBaseCell* pCell = aCol[nCol].GetCell(nRow);
            if (!pCell)
            {
                // empty cell found
                rMark.SetMultiMarkArea(ScRange(nCol, nRow, nTab));
                bFound = true;

                if (bReplace)
                {
                    aCol[nCol].Insert(nRow, new ScStringCell(rSearchItem.GetReplaceString()));
                    if (pUndoDoc)
                        // TODO: I'm using a string cell with empty content to
                        // trigger deletion of cell instance on undo.  Maybe I
                        // should create a new cell type for this?
                        pUndoDoc->PutCell(nCol, nRow, nTab, new ScStringCell(String()));
                }
            }
            else if (pCell->GetCellType() == CELLTYPE_NOTE)
            {
                rMark.SetMultiMarkArea(ScRange(nCol, nRow, nTab));
                bFound = true;

                if (bReplace)
                {
                    if (pUndoDoc)
                    {
                        ScAddress aCellPos(nCol, nRow, nTab);
                        pUndoDoc->PutCell(nCol, nRow, nTab, pCell->CloneWithNote(aCellPos, *pUndoDoc, aCellPos));
                    }
                    aCol[nCol].SetString(nRow, nTab, rSearchItem.GetReplaceString());
                }
            }
        }
    }
    return bFound;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
