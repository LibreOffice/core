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

#ifndef SC_COLUMN_HXX
#define SC_COLUMN_HXX

#include "markarr.hxx"
#include "global.hxx"
#include "address.hxx"
#include "rangenam.hxx"
#include <tools/solar.h>

#include <set>

class Fraction;
class OutputDevice;
class Rectangle;
class SfxBroadcaster;
class SfxItemPoolCache;
class SfxItemSet;
class SvtListener;
class SfxPoolItem;
class SfxStyleSheetBase;
class SvxBorderLine;
class SvxBoxInfoItem;
class SvxBoxItem;

class ScAttrIterator;
class ScAttrArray;
class ScBaseCell;
class ScDocument;
class ScFormulaCell;
class ScMarkData;
class ScPatternAttr;
class ScStyleSheet;
class SvtBroadcaster;
class TypedScStrCollection;
class ScProgress;
class ScPostIt;
struct ScFunctionData;
struct ScLineFlags;
struct ScMergePatternState;
class ScFlatBoolRowSegments;

#define COLUMN_DELTA    4


struct ScNeededSizeOptions
{
    const ScPatternAttr*    pPattern;
    sal_Bool                    bFormula;
    sal_Bool                    bSkipMerged;
    sal_Bool                    bGetFont;
    sal_Bool                    bTotalSize;

    ScNeededSizeOptions()
    {
        pPattern = NULL;
        bFormula = sal_False;
        bSkipMerged = sal_True;
        bGetFont = sal_True;
        bTotalSize = sal_False;
    }
};

struct ColEntry
{
    SCROW       nRow;
    ScBaseCell* pCell;
};


class ScColumn
{
private:
    SCCOL           nCol;
    SCTAB           nTab;

    SCSIZE          nCount;
    SCSIZE          nLimit;
    ColEntry*       pItems;

    ScAttrArray*    pAttrArray;
    ScDocument*     pDocument;

friend class ScDocument;                    // fuer FillInfo
friend class ScDocumentIterator;
friend class ScValueIterator;
friend class ScHorizontalValueIterator;
friend class ScDBQueryDataIterator;
friend class ScColumnIterator;
friend class ScQueryCellIterator;
friend class ScMarkedDataIter;
friend class ScCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;

public:
static sal_Bool bDoubleAlloc;           // fuer Import: Groesse beim Allozieren verdoppeln

public:
                ScColumn();
                ~ScColumn();

    void        Init(SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc);

    sal_Bool        Search( SCROW nRow, SCSIZE& nIndex ) const;
    ScBaseCell* GetCell( SCROW nRow ) const;
    void        Insert( SCROW nRow, ScBaseCell* pCell );
    void        Insert( SCROW nRow, sal_uLong nFormatIndex, ScBaseCell* pCell );
    void        Append( SCROW nRow, ScBaseCell* pCell );
    void        Delete( SCROW nRow );
    void        DeleteAtIndex( SCSIZE nIndex );
    void        FreeAll();
    void        Resize( SCSIZE nSize );
    void        SwapRow( SCROW nRow1, SCROW nRow2 );
    void        SwapCell( SCROW nRow, ScColumn& rCol);

//UNUSED2009-05 sal_Bool        HasLines( SCROW nRow1, SCROW nRow2, Rectangle& rSizes,
//UNUSED2009-05             sal_Bool bLeft, sal_Bool bRight ) const;
    bool        HasAttrib( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const;
    sal_Bool        HasAttribSelection( const ScMarkData& rMark, sal_uInt16 nMask ) const;
    sal_Bool        ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                sal_Bool bRefresh, sal_Bool bAttrs );

    sal_Bool        IsEmptyVisData(sal_Bool bNotes) const;      // ohne Broadcaster
    sal_Bool        IsEmptyData() const;
    sal_Bool        IsEmptyAttr() const;
    sal_Bool        IsEmpty() const;

                // nur Daten:
    sal_Bool        IsEmptyBlock(SCROW nStartRow, SCROW nEndRow, bool bIgnoreNotes = false) const;
    SCSIZE      GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const;
    sal_Bool        HasDataAt(SCROW nRow) const;
    sal_Bool        HasVisibleDataAt(SCROW nRow) const;
    SCROW       GetFirstDataPos() const;
    SCROW       GetLastDataPos() const;
    SCROW       GetLastVisDataPos(sal_Bool bNotes) const;               // ohne Broadcaster
    SCROW       GetFirstVisDataPos(sal_Bool bNotes) const;
    sal_Bool        GetPrevDataPos(SCROW& rRow) const;
    sal_Bool        GetNextDataPos(SCROW& rRow) const;
    void        FindDataAreaPos(SCROW& rRow, long nMovY) const; // (ohne Broadcaster)
    void        FindUsed( SCROW nStartRow, SCROW nEndRow, sal_Bool* pUsed ) const;

    SCSIZE      VisibleCount( SCROW nStartRow, SCROW nEndRow ) const;

    sal_uInt16      GetBlockMatrixEdges( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const;
    sal_Bool        HasSelectionMatrixFragment(const ScMarkData& rMark) const;

    sal_Bool        GetFirstVisibleAttr( SCROW& rFirstRow ) const;
    sal_Bool        GetLastVisibleAttr( SCROW& rLastRow ) const;
    sal_Bool        HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const;
    sal_Bool        IsVisibleAttrEqual( const ScColumn& rCol, SCROW nStartRow = 0,
                                    SCROW nEndRow = MAXROW ) const;
    sal_Bool        IsAllAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const;

    sal_Bool        TestInsertCol( SCROW nStartRow, SCROW nEndRow) const;
    sal_Bool        TestInsertRow( SCSIZE nSize ) const;
    void        InsertRow( SCROW nStartRow, SCSIZE nSize );
    void        DeleteRow( SCROW nStartRow, SCSIZE nSize );
    void        DeleteRange( SCSIZE nStartIndex, SCSIZE nEndIndex, sal_uInt16 nDelFlag );
    void        DeleteArea(SCROW nStartRow, SCROW nEndRow, sal_uInt16 nDelFlag );
    void        CopyToClip(SCROW nRow1, SCROW nRow2, ScColumn& rColumn, sal_Bool bKeepScenarioFlags, sal_Bool bCloneNoteCaptions);
    void        CopyFromClip(SCROW nRow1, SCROW nRow2, long nDy,
                                sal_uInt16 nInsFlag, sal_Bool bAsLink, sal_Bool bSkipAttrForEmpty, ScColumn& rColumn);
    void        StartListeningInArea( SCROW nRow1, SCROW nRow2 );
    void        BroadcastInArea( SCROW nRow1, SCROW nRow2 );

    void        RemoveEditAttribs( SCROW nStartRow, SCROW nEndRow );

                //  Markierung von diesem Dokument
    void        MixMarked( const ScMarkData& rMark, sal_uInt16 nFunction,
                            sal_Bool bSkipEmpty, ScColumn& rSrcCol );
    void        MixData( SCROW nRow1, SCROW nRow2, sal_uInt16 nFunction, sal_Bool bSkipEmpty,
                            ScColumn& rSrcCol );

    ScFormulaCell*  CreateRefCell( ScDocument* pDestDoc, const ScAddress& rDestPos,
                                    SCSIZE nIndex, sal_uInt16 nFlags ) const;

    ScAttrIterator* CreateAttrIterator( SCROW nStartRow, SCROW nEndRow ) const;

    SCCOL       GetCol() const { return nCol; }

                //  UpdateSelectionFunction: Mehrfachselektion
    void        UpdateSelectionFunction( const ScMarkData& rMark,
                                    ScFunctionData& rData,
                                    ScFlatBoolRowSegments& rHiddenRows,
                                    sal_Bool bDoExclude, SCROW nExStartRow, SCROW nExEndRow );
    void        UpdateAreaFunction( ScFunctionData& rData,
                                    ScFlatBoolRowSegments& rHiddenRows,
                                    SCROW nStartRow, SCROW nEndRow );

    void        CopyToColumn(SCROW nRow1, SCROW nRow2, sal_uInt16 nFlags, sal_Bool bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData = NULL,
                                sal_Bool bAsLink = sal_False );
    void        UndoToColumn(SCROW nRow1, SCROW nRow2, sal_uInt16 nFlags, sal_Bool bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData = NULL );

    void        CopyScenarioFrom( const ScColumn& rSrcCol );
    void        CopyScenarioTo( ScColumn& rDestCol ) const;
    sal_Bool        TestCopyScenarioTo( const ScColumn& rDestCol ) const;
    void        MarkScenarioIn( ScMarkData& rDestMark ) const;

    void        CopyUpdated( const ScColumn& rPosCol, ScColumn& rDestCol ) const;

    void        SwapCol(ScColumn& rCol);
    void        MoveTo(SCROW nStartRow, SCROW nEndRow, ScColumn& rCol);

    sal_Bool        HasEditCells(SCROW nStartRow, SCROW nEndRow, SCROW& rFirst) const;

                //  sal_True = Zahlformat gesetzt
    sal_Bool        SetString( SCROW nRow, SCTAB nTab, const String& rString,
                           formula::FormulaGrammar::AddressConvention conv = formula::FormulaGrammar::CONV_OOO,
                           SvNumberFormatter* pFormatter = NULL,
                           bool bDetectNumberFormat = true );
    void        SetValue( SCROW nRow, const double& rVal);
    void        SetError( SCROW nRow, const sal_uInt16 nError);

    void        GetString( SCROW nRow, String& rString ) const;
    void        GetInputString( SCROW nRow, String& rString ) const;
    double      GetValue( SCROW nRow ) const;
    void        GetFormula( SCROW nRow, String& rFormula,
                            sal_Bool bAsciiExport = sal_False ) const;
    CellType    GetCellType( SCROW nRow ) const;
    SCSIZE      GetCellCount() const { return nCount; }
    sal_uLong       GetWeightedCount() const;
    sal_uLong       GetCodeCount() const;       // RPN-Code in Formeln
    sal_uInt16      GetErrCode( SCROW nRow ) const;

    sal_Bool        HasStringData( SCROW nRow ) const;
    sal_Bool        HasValueData( SCROW nRow ) const;
//UNUSED2009-05 sal_uInt16      GetErrorData( SCROW nRow) const;
    sal_Bool        HasStringCells( SCROW nStartRow, SCROW nEndRow ) const;

    /** Returns the pointer to a cell note object at the passed row. */
    ScPostIt*   GetNote( SCROW nRow );
    /** Sets the passed cell note object at the passed row. Takes ownership! */
    void        TakeNote( SCROW nRow, ScPostIt* pNote );
    /** Returns and forgets a cell note object at the passed row. */
    ScPostIt*   ReleaseNote( SCROW nRow );
    /** Deletes the note at the passed row. */
    void        DeleteNote( SCROW nRow );

    void        SetDirty();
    void        SetDirty( const ScRange& );
    void        SetDirtyVar();
    void        SetDirtyAfterLoad();
    void        SetTableOpDirty( const ScRange& );
    void        CalcAll();
    void        CalcAfterLoad();
    void        CompileAll();
    void        CompileXML( ScProgress& rProgress );

    void        ResetChanged( SCROW nStartRow, SCROW nEndRow );

    void        UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                     SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                     ScDocument* pUndoDoc = NULL );
    void        UpdateInsertTab( SCTAB nTable);
    void        UpdateInsertTabOnlyCells( SCTAB nTable);
    void        UpdateDeleteTab( SCTAB nTable, sal_Bool bIsMove, ScColumn* pRefUndo = NULL );
    void        UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo);
    void        UpdateCompile( sal_Bool bForceIfNameInUse = sal_False );
    void        UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc );
    void        UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void        SetTabNo(SCTAB nNewTab);
    sal_Bool        IsRangeNameInUse(SCROW nRow1, SCROW nRow2, sal_uInt16 nIndex) const;
    void        FindRangeNamesInUse(SCROW nRow1, SCROW nRow2, std::set<sal_uInt16>& rIndexes) const;
    void        ReplaceRangeNamesInUse( SCROW nRow1, SCROW nRow2, const ScRangeData::IndexMap& rMap );

    const SfxPoolItem*      GetAttr( SCROW nRow, sal_uInt16 nWhich ) const;
    const ScPatternAttr*    GetPattern( SCROW nRow ) const;
    const ScPatternAttr*    GetMostUsedPattern( SCROW nStartRow, SCROW nEndRow ) const;

    sal_uLong       GetNumberFormat( SCROW nRow ) const;

    void        MergeSelectionPattern( ScMergePatternState& rState, const ScMarkData& rMark, sal_Bool bDeep ) const;
    void        MergePatternArea( ScMergePatternState& rState, SCROW nRow1, SCROW nRow2, sal_Bool bDeep ) const;
    void        MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                            ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, sal_Bool bLeft, SCCOL nDistRight ) const;
    void        ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow, sal_Bool bLeft, SCCOL nDistRight );

    void        ApplyAttr( SCROW nRow, const SfxPoolItem& rAttr );
    void        ApplyPattern( SCROW nRow, const ScPatternAttr& rPatAttr );
    void        ApplyPatternArea( SCROW nStartRow, SCROW nEndRow, const ScPatternAttr& rPatAttr );
    void        SetPattern( SCROW nRow, const ScPatternAttr& rPatAttr, sal_Bool bPutToPool = sal_False );
    void        SetPatternArea( SCROW nStartRow, SCROW nEndRow,
                                const ScPatternAttr& rPatAttr, sal_Bool bPutToPool = sal_False );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, short nNewType );

    void        ApplyStyle( SCROW nRow, const ScStyleSheet& rStyle );
    void        ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, const ScStyleSheet& rStyle );
    void        ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);
    void        ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const SvxBorderLine* pLine, sal_Bool bColorOnly );

    const ScStyleSheet* GetStyle( SCROW nRow ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark, sal_Bool& rFound ) const;
    const ScStyleSheet* GetAreaStyle( sal_Bool& rFound, SCROW nRow1, SCROW nRow2 ) const;

    void        FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset );
    sal_Bool        IsStyleSheetUsed( const ScStyleSheet& rStyle, sal_Bool bGatherAllStyles ) const;

                /// May return -1 if not found
    SCsROW      SearchStyle( SCsROW nRow, const ScStyleSheet* pSearchStyle,
                                sal_Bool bUp, sal_Bool bInSelection, const ScMarkData& rMark );
    sal_Bool        SearchStyleRange( SCsROW& rRow, SCsROW& rEndRow, const ScStyleSheet* pSearchStyle,
                                    sal_Bool bUp, sal_Bool bInSelection, const ScMarkData& rMark );

    sal_Bool        ApplyFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags );
    sal_Bool        RemoveFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags );
    void        ClearItems( SCROW nStartRow, SCROW nEndRow, const sal_uInt16* pWhich );

    void        RemoveProtected( SCROW nStartRow, SCROW nEndRow );

    SCsROW      ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark );
    void        DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark );

    void        ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark );
    void        ChangeSelectionIndent( sal_Bool bIncrement, const ScMarkData& rMark );

    long        GetNeededSize( SCROW nRow, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    sal_Bool bWidth, const ScNeededSizeOptions& rOptions );
    sal_uInt16      GetOptimalColWidth( OutputDevice* pDev, double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    sal_Bool bFormula, sal_uInt16 nOldWidth,
                                    const ScMarkData* pMarkData,
                                    sal_Bool bSimpleTextImport );
    void        GetOptimalHeight( SCROW nStartRow, SCROW nEndRow, sal_uInt16* pHeight,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    sal_Bool bShrink, sal_uInt16 nMinHeight, SCROW nMinStart );
private:
    long        GetSimpleTextNeededSize( SCSIZE nIndex, OutputDevice* pDev,
                                    sal_Bool bWidth );
public:

                /// Including current, may return -1
    SCsROW      GetNextUnprotected( SCROW nRow, sal_Bool bUp ) const;

    void        GetFilterEntries(SCROW nStartRow, SCROW nEndRow, TypedScStrCollection& rStrings, bool& rHasDates);
    sal_Bool        GetDataEntries(SCROW nRow, TypedScStrCollection& rStrings, sal_Bool bLimit);

//UNUSED2008-05  SCROW      NoteCount( SCROW nMaxRow = MAXROW ) const;

    void        UpdateInsertTabAbs(SCTAB nNewPos);
    sal_Bool        TestTabRefAbs(SCTAB nTable);
    sal_Bool        GetNextSpellingCell(SCROW& nRow, sal_Bool bInSel, const ScMarkData& rData) const;

    void        RemoveAutoSpellObj();

    void        StartListening( SvtListener& rLst, SCROW nRow );
    void        EndListening( SvtListener& rLst, SCROW nRow );
    void        MoveListeners( SvtBroadcaster& rSource, SCROW nDestRow );
    void        StartAllListeners();
    void        StartNeededListeners(); // only for cells where NeedsListening()==TRUE
    void        SetRelNameDirty();

    void        CompileDBFormula();
    void        CompileDBFormula( sal_Bool bCreateFormulaString );
    void        CompileNameFormula( sal_Bool bCreateFormulaString );
    void        CompileColRowNameFormula();

    sal_Int32   GetMaxStringLen( SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const;
    xub_StrLen  GetMaxNumberStringLen( sal_uInt16& nPrecision,
                                       SCROW nRowStart, SCROW nRowEnd ) const;

private:
    ScBaseCell* CloneCell(SCSIZE nIndex, sal_uInt16 nFlags, ScDocument& rDestDoc, const ScAddress& rDestPos);
//UNUSED2008-05  void       CorrectSymbolCells( CharSet eStreamCharSet );
};


class ScColumnIterator                  // alle Daten eines Bereichs durchgehen
{
    const ScColumn*     pColumn;
    SCSIZE              nPos;
    SCROW               nTop;
    SCROW               nBottom;
public:
                ScColumnIterator( const ScColumn* pCol, SCROW nStart=0, SCROW nEnd=MAXROW );
                ~ScColumnIterator();

    sal_Bool        Next( SCROW& rRow, ScBaseCell*& rpCell );
    SCSIZE      GetIndex() const;
};


class ScMarkedDataIter                  // Daten in selektierten Bereichen durchgehen
{
    const ScColumn*     pColumn;
    SCSIZE              nPos;
    ScMarkArrayIter*    pMarkIter;
    SCROW               nTop;
    SCROW               nBottom;
    sal_Bool                bNext;
    sal_Bool                bAll;

public:
                ScMarkedDataIter( const ScColumn* pCol, const ScMarkData* pMarkData,
                                    sal_Bool bAllIfNone = sal_False );
                ~ScMarkedDataIter();

    sal_Bool        Next( SCSIZE& rIndex );
};


#endif


