/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


sal_Bool lcl_GetTextWithBreaks( const ScEditCell& rCell, ScDocument* pDoc, String& rVal )
{
    //  sal_True = more than 1 paragraph

    const EditTextObject* pData = NULL;
    rCell.GetData( pData );
    EditEngine& rEngine = pDoc->GetEditEngine();
    rEngine.SetText( *pData );
    rVal = rEngine.GetText( LINEEND_LF );
    return ( rEngine.GetParagraphCount() > 1 );
}

sal_Bool ScTable::SearchCell(const SvxSearchItem& rSearchItem, SCCOL nCol, SCROW nRow,
                            const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc)
{
    sal_Bool    bFound = sal_False;
    sal_Bool    bDoSearch = sal_True;
    sal_Bool    bDoBack = rSearchItem.GetBackward();

    String  aString;
    ScBaseCell* pCell;
    if (rSearchItem.GetSelection())
        bDoSearch = rMark.IsCellMarked(nCol, nRow);
    if ( bDoSearch && ((pCell = aCol[nCol].GetCell( nRow )) != NULL) )
    {
        sal_Bool bMultiLine = sal_False;
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
                    aCol[nCol].GetString( nRow, aString );
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
                bFound = (sal_Bool)(pSearchText->SearchBkwrd(aString, &nStart, &nEnd, &aSearchResult));
                // change results to definition before 614:
                --nEnd;
            }
            else
            {
                bFound = (sal_Bool)(pSearchText->SearchFrwrd(aString, &nStart, &nEnd, &aSearchResult));
                // change results to definition before 614:
                --nEnd;
            }

            if (bFound && rSearchItem.GetWordOnly())
                bFound = (nStart == 0 && nEnd == aString.Len() - 1);
        }
        else
        {
            DBG_ERROR("pSearchText == NULL");
            return bFound;
        }

        sal_uInt8 cMatrixFlag = MM_NONE;
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
            sal_Bool bRepeat = !rSearchItem.GetWordOnly();
            do
            {
                //  wenn der gefundene Text leer ist, nicht weitersuchen,
                //  sonst wuerde man nie mehr aufhoeren (#35410#)
                if ( nEnd < nStart || nEnd == STRING_MAXLEN )
                    bRepeat = sal_False;

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
                        bRepeat = sal_False;
                    else if (bDoBack)
                    {
                        xub_StrLen nTemp=nStart; nStart=nEnd; nEnd=nTemp;
                        bRepeat = ((sal_Bool)(pSearchText->SearchBkwrd(aString, &nStart, &nEnd, &aSearchResult)));
                        // change results to definition before 614:
                        --nEnd;
                    }
                    else
                    {
                        bRepeat = ((sal_Bool)(pSearchText->SearchFrwrd(aString, &nStart, &nEnd, &aSearchResult)));
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

sal_Bool ScTable::Search(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc)
{
    sal_Bool bFound = sal_False;
    sal_Bool bAll =  (rSearchItem.GetCommand() == SVX_SEARCHCMD_FIND_ALL)
               ||(rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL);
    SCCOL nCol = rCol;
    SCROW nRow = rRow;
    SCCOL nLastCol;
    SCROW nLastRow;
    GetLastDataPos(nLastCol, nLastRow);
    if (!bAll && rSearchItem.GetBackward())
    {
        nCol = Min(nCol, (SCCOL)(nLastCol + 1));
        nRow = Min(nRow, (SCROW)(nLastRow + 1));
        if (rSearchItem.GetRowDirection())
        {
            nCol--;
            while (!bFound && ((SCsROW)nRow >= 0))
            {
                while (!bFound && ((SCsCOL)nCol >= 0))
                {
                    bFound = SearchCell(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                        sal_Bool bIsEmpty;
                        do
                        {
                            nCol--;
                            if ((SCsCOL)nCol >= 0)
                                bIsEmpty = aCol[nCol].IsEmptyData();
                            else
                                bIsEmpty = sal_True;
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
                    bFound = SearchCell(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                         if (!aCol[nCol].GetPrevDataPos(nRow))
                            nRow = -1;
                    }
                }
                if (!bFound)
                {
                    sal_Bool bIsEmpty;
                    nRow = nLastRow;
                    do
                    {
                        nCol--;
                        if ((SCsCOL)nCol >= 0)
                            bIsEmpty = aCol[nCol].IsEmptyData();
                        else
                            bIsEmpty = sal_True;
                    }
                    while (((SCsCOL)nCol >= 0) && bIsEmpty);
                }
            }
        }
    }
    else
    {
        if (!bAll && rSearchItem.GetRowDirection())
        {
            nCol++;
            while (!bFound && (nRow <= nLastRow))
            {
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
                    bFound = SearchCell(rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc);
                    if (!bFound)
                    {
                         if (!aCol[nCol].GetNextDataPos(nRow))
                            nRow = MAXROW + 1;
                    }
                }
                if (!bFound)
                {
                    nRow = 0;
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

sal_Bool ScTable::SearchAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                        String& rUndoStr, ScDocument* pUndoDoc)
{
    sal_Bool bFound = sal_True;
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

sal_Bool ScTable::Replace(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc)
{
    sal_Bool bFound = sal_False;
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

sal_Bool ScTable::ReplaceAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                            String& rUndoStr, ScDocument* pUndoDoc)
{
    sal_Bool bOldDouble = ScColumn::bDoubleAlloc;       // sollte immer sal_False sein?
    DBG_ASSERT(!bOldDouble,"bDoubleAlloc ???");
    ScColumn::bDoubleAlloc = sal_True;                  // fuer Undo-Doc

    sal_Bool bFound = sal_True;
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

sal_Bool ScTable::SearchStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                            ScMarkData& rMark)
{
    const ScStyleSheet* pSearchStyle = (const ScStyleSheet*)
                                        pDocument->GetStyleSheetPool()->Find(
                                        rSearchItem.GetSearchString(), SFX_STYLE_FAMILY_PARA );

    SCsCOL nCol = rCol;
    SCsROW nRow = rRow;
    sal_Bool bFound = sal_False;

    sal_Bool bSelect = rSearchItem.GetSelection();
    sal_Bool bRows = rSearchItem.GetRowDirection();
    sal_Bool bBack = rSearchItem.GetBackward();
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
                bFound = sal_True;
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
                    bFound = sal_True;
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
                    bFound = sal_True;
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

sal_Bool ScTable::ReplaceStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                           ScMarkData& rMark, sal_Bool bIsUndo)
{
    sal_Bool bRet;
    if (bIsUndo)
        bRet = sal_True;
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
            DBG_ERROR("pReplaceStyle==0");
        }
    }

    return bRet;
}

sal_Bool ScTable::SearchAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark)
{
    const ScStyleSheet* pSearchStyle = (const ScStyleSheet*)
                                        pDocument->GetStyleSheetPool()->Find(
                                        rSearchItem.GetSearchString(), SFX_STYLE_FAMILY_PARA );
    sal_Bool bSelect = rSearchItem.GetSelection();
    sal_Bool bBack = rSearchItem.GetBackward();

    ScMarkData aNewMark( rMark );   // Tabellen-Markierungen kopieren
    aNewMark.ResetMark();
    for (SCCOL i=0; i<=MAXCOL; i++)
    {
        sal_Bool bFound = sal_True;
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

sal_Bool ScTable::ReplaceAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                                ScDocument* pUndoDoc)
{
    sal_Bool bRet = SearchAllStyle(rSearchItem, rMark);
    if (bRet)
    {
        const ScStyleSheet* pReplaceStyle = (const ScStyleSheet*)
                                        pDocument->GetStyleSheetPool()->Find(
                                        rSearchItem.GetReplaceString(), SFX_STYLE_FAMILY_PARA );

        if (pReplaceStyle)
        {
            if (pUndoDoc)
                pDocument->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                            IDF_ATTRIB, sal_True, pUndoDoc, &rMark );
            ApplySelectionStyle( *pReplaceStyle, rMark );
        }
        else
        {
            DBG_ERROR("pReplaceStyle==0");
        }
    }

    return bRet;
}

sal_Bool ScTable::SearchAndReplace(const SvxSearchItem& rSearchItem,
                                SCCOL& rCol, SCROW& rRow, ScMarkData& rMark,
                                String& rUndoStr, ScDocument* pUndoDoc)
{
    sal_uInt16 nCommand = rSearchItem.GetCommand();
    sal_Bool bFound = sal_False;
    if ( ValidColRow(rCol, rRow) ||
         ((nCommand == SVX_SEARCHCMD_FIND || nCommand == SVX_SEARCHCMD_REPLACE) &&
           (((rCol == MAXCOLCOUNT || rCol == -1) && VALIDROW(rRow)) ||
            ((rRow == MAXROWCOUNT || rRow == -1) && VALIDCOL(rCol))
           )
         )
       )
    {
        sal_Bool bStyles = rSearchItem.GetPattern();
        if (bStyles)
        {
            if (nCommand == SVX_SEARCHCMD_FIND)
                bFound = SearchStyle(rSearchItem, rCol, rRow, rMark);
            else if (nCommand == SVX_SEARCHCMD_REPLACE)
                bFound = ReplaceStyle(rSearchItem, rCol, rRow, rMark, sal_False);
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






