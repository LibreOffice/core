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
struct ScAttrEntry;
class ScBaseCell;
class ScDocument;
class ScEditDataArray;
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
struct ScSetStringParam;
struct ScColWidthParam;

#define COLUMN_DELTA    4


struct ScNeededSizeOptions
{
    const ScPatternAttr*    pPattern;
    BOOL                    bFormula;
    BOOL                    bSkipMerged;
    BOOL                    bGetFont;
    BOOL                    bTotalSize;

    ScNeededSizeOptions()
    {
        pPattern = NULL;
        bFormula = FALSE;
        bSkipMerged = TRUE;
        bGetFont = TRUE;
        bTotalSize = FALSE;
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
friend class ScDBQueryDataIterator;
friend class ScColumnIterator;
friend class ScQueryCellIterator;
friend class ScMarkedDataIter;
friend class ScCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;

public:
    static bool bDoubleAlloc;           // fuer Import: Groesse beim Allozieren verdoppeln

    class DoubleAllocSwitch
    {
    public:
        DoubleAllocSwitch(bool bNewVal = true);
        ~DoubleAllocSwitch();
    private:
        bool mbOldVal;
    };
public:
                ScColumn();
                ~ScColumn();

    void        Init(SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc);

    BOOL        Search( SCROW nRow, SCSIZE& nIndex ) const;
    ScBaseCell* GetCell( SCROW nRow ) const;
    void        Insert( SCROW nRow, ScBaseCell* pCell );
    void        Insert( SCROW nRow, ULONG nFormatIndex, ScBaseCell* pCell );
    void        Append( SCROW nRow, ScBaseCell* pCell );
    void        Delete( SCROW nRow );
    void        DeleteAtIndex( SCSIZE nIndex );
    void        FreeAll();
    void        Resize( SCSIZE nSize );
    void        SwapRow( SCROW nRow1, SCROW nRow2 );
    void        SwapCell( SCROW nRow, ScColumn& rCol);

    bool        HasAttrib( SCROW nRow1, SCROW nRow2, USHORT nMask ) const;
    BOOL        HasAttribSelection( const ScMarkData& rMark, USHORT nMask ) const;
    BOOL        ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                BOOL bRefresh, BOOL bAttrs );

    BOOL        IsEmptyVisData(BOOL bNotes) const;      // ohne Broadcaster
    BOOL        IsEmptyData() const;
    BOOL        IsEmptyAttr() const;
    BOOL        IsEmpty() const;

                // nur Daten:
    BOOL        IsEmptyBlock(SCROW nStartRow, SCROW nEndRow, bool bIgnoreNotes = false) const;
    SCSIZE      GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const;
    BOOL        HasDataAt(SCROW nRow) const;
    BOOL        HasVisibleDataAt(SCROW nRow) const;
    SCROW       GetFirstDataPos() const;
    SCROW       GetLastDataPos() const;
    SCROW       GetLastVisDataPos(BOOL bNotes) const;               // ohne Broadcaster
    SCROW       GetFirstVisDataPos(BOOL bNotes) const;
    BOOL        GetPrevDataPos(SCROW& rRow) const;
    BOOL        GetNextDataPos(SCROW& rRow) const;
    void        FindDataAreaPos(SCROW& rRow, long nMovY) const; // (ohne Broadcaster)
    void        FindUsed( SCROW nStartRow, SCROW nEndRow, BOOL* pUsed ) const;

    SCSIZE      VisibleCount( SCROW nStartRow, SCROW nEndRow ) const;

    USHORT      GetBlockMatrixEdges( SCROW nRow1, SCROW nRow2, USHORT nMask ) const;
    BOOL        HasSelectionMatrixFragment(const ScMarkData& rMark) const;

    BOOL        GetFirstVisibleAttr( SCROW& rFirstRow ) const;
    BOOL        GetLastVisibleAttr( SCROW& rLastRow ) const;
    BOOL        HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const;
    BOOL        IsVisibleAttrEqual( const ScColumn& rCol, SCROW nStartRow = 0,
                                    SCROW nEndRow = MAXROW ) const;
    BOOL        IsAllAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const;

    BOOL        TestInsertCol( SCROW nStartRow, SCROW nEndRow) const;
    BOOL        TestInsertRow( SCSIZE nSize ) const;
    void        InsertRow( SCROW nStartRow, SCSIZE nSize );
    void        DeleteRow( SCROW nStartRow, SCSIZE nSize );
    void        DeleteRange( SCSIZE nStartIndex, SCSIZE nEndIndex, USHORT nDelFlag );
    void        DeleteArea(SCROW nStartRow, SCROW nEndRow, USHORT nDelFlag );
    void        CopyToClip(SCROW nRow1, SCROW nRow2, ScColumn& rColumn, BOOL bKeepScenarioFlags, BOOL bCloneNoteCaptions);
    void        CopyFromClip(SCROW nRow1, SCROW nRow2, long nDy,
                                USHORT nInsFlag, BOOL bAsLink, BOOL bSkipAttrForEmpty, ScColumn& rColumn);
    void        StartListeningInArea( SCROW nRow1, SCROW nRow2 );
    void        BroadcastInArea( SCROW nRow1, SCROW nRow2 );

    void        RemoveEditAttribs( SCROW nStartRow, SCROW nEndRow );

                //  Markierung von diesem Dokument
    void        MixMarked( const ScMarkData& rMark, USHORT nFunction,
                            BOOL bSkipEmpty, ScColumn& rSrcCol );
    void        MixData( SCROW nRow1, SCROW nRow2, USHORT nFunction, BOOL bSkipEmpty,
                            ScColumn& rSrcCol );

    ScFormulaCell*  CreateRefCell( ScDocument* pDestDoc, const ScAddress& rDestPos,
                                    SCSIZE nIndex, USHORT nFlags ) const;

    ScAttrIterator* CreateAttrIterator( SCROW nStartRow, SCROW nEndRow ) const;

    SCCOL       GetCol() const { return nCol; }

                //  UpdateSelectionFunction: Mehrfachselektion
    void        UpdateSelectionFunction( const ScMarkData& rMark,
                                    ScFunctionData& rData,
                                    ScFlatBoolRowSegments& rHiddenRows,
                                    BOOL bDoExclude, SCROW nExStartRow, SCROW nExEndRow );
    void        UpdateAreaFunction( ScFunctionData& rData,
                                    ScFlatBoolRowSegments& rHiddenRows,
                                    SCROW nStartRow, SCROW nEndRow );

    void        CopyToColumn(SCROW nRow1, SCROW nRow2, USHORT nFlags, BOOL bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData = NULL,
                                BOOL bAsLink = FALSE );
    void        UndoToColumn(SCROW nRow1, SCROW nRow2, USHORT nFlags, BOOL bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData = NULL );

    void        CopyScenarioFrom( const ScColumn& rSrcCol );
    void        CopyScenarioTo( ScColumn& rDestCol ) const;
    BOOL        TestCopyScenarioTo( const ScColumn& rDestCol ) const;
    void        MarkScenarioIn( ScMarkData& rDestMark ) const;

    void        CopyUpdated( const ScColumn& rPosCol, ScColumn& rDestCol ) const;

    void        SwapCol(ScColumn& rCol);
    void        MoveTo(SCROW nStartRow, SCROW nEndRow, ScColumn& rCol);

    BOOL        HasEditCells(SCROW nStartRow, SCROW nEndRow, SCROW& rFirst) const;

                //  TRUE = Zahlformat gesetzt
    BOOL        SetString( SCROW nRow, SCTAB nTab, const String& rString,
                           formula::FormulaGrammar::AddressConvention conv = formula::FormulaGrammar::CONV_OOO,
                           ScSetStringParam* pParam = NULL );
    void        SetValue( SCROW nRow, const double& rVal);
    void        SetError( SCROW nRow, const USHORT nError);

    void        GetString( SCROW nRow, String& rString ) const;
    void        GetInputString( SCROW nRow, String& rString ) const;
    double      GetValue( SCROW nRow ) const;
    void        GetFormula( SCROW nRow, String& rFormula,
                            BOOL bAsciiExport = FALSE ) const;
    CellType    GetCellType( SCROW nRow ) const;
    SCSIZE      GetCellCount() const { return nCount; }
    ULONG       GetWeightedCount() const;
    ULONG       GetCodeCount() const;       // RPN-Code in Formeln
    USHORT      GetErrCode( SCROW nRow ) const;

    BOOL        HasStringData( SCROW nRow ) const;
    BOOL        HasValueData( SCROW nRow ) const;
    BOOL        HasStringCells( SCROW nStartRow, SCROW nEndRow ) const;

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

    bool        UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                     SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                     ScDocument* pUndoDoc = NULL );
    void        UpdateInsertTab( SCTAB nTable);
    void        UpdateInsertTabOnlyCells( SCTAB nTable);
    void        UpdateDeleteTab( SCTAB nTable, BOOL bIsMove, ScColumn* pRefUndo = NULL );
    void        UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo);
    void        UpdateCompile( BOOL bForceIfNameInUse = FALSE );
    void        UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc );
    void        UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void        SetTabNo(SCTAB nNewTab);
    BOOL        IsRangeNameInUse(SCROW nRow1, SCROW nRow2, USHORT nIndex) const;
    void        FindRangeNamesInUse(SCROW nRow1, SCROW nRow2, std::set<USHORT>& rIndexes) const;
    void        ReplaceRangeNamesInUse( SCROW nRow1, SCROW nRow2, const ScRangeData::IndexMap& rMap );

    const SfxPoolItem*      GetAttr( SCROW nRow, USHORT nWhich ) const;
    const ScPatternAttr*    GetPattern( SCROW nRow ) const;
    const ScPatternAttr*    GetMostUsedPattern( SCROW nStartRow, SCROW nEndRow ) const;

    ULONG       GetNumberFormat( SCROW nRow ) const;
    sal_uInt32  GetNumberFormat( SCROW nStartRow, SCROW nEndRow ) const;

    void        MergeSelectionPattern( ScMergePatternState& rState, const ScMarkData& rMark, BOOL bDeep ) const;
    void        MergePatternArea( ScMergePatternState& rState, SCROW nRow1, SCROW nRow2, BOOL bDeep ) const;
    void        MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                            ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, BOOL bLeft, SCCOL nDistRight ) const;
    void        ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow, BOOL bLeft, SCCOL nDistRight );

    void        ApplyAttr( SCROW nRow, const SfxPoolItem& rAttr );
    void        ApplyPattern( SCROW nRow, const ScPatternAttr& rPatAttr );
    void        ApplyPatternArea( SCROW nStartRow, SCROW nEndRow, const ScPatternAttr& rPatAttr,
                                  ScEditDataArray* pDataArray = NULL );
    bool        SetAttrEntries(ScAttrEntry* pData, SCSIZE nSize);
    void        SetPattern( SCROW nRow, const ScPatternAttr& rPatAttr, BOOL bPutToPool = FALSE );
    void        SetPatternArea( SCROW nStartRow, SCROW nEndRow,
                                const ScPatternAttr& rPatAttr, BOOL bPutToPool = FALSE );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, short nNewType );

    void        ApplyStyle( SCROW nRow, const ScStyleSheet& rStyle );
    void        ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, const ScStyleSheet& rStyle );
    void        ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);
    void        ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const SvxBorderLine* pLine, BOOL bColorOnly );

    const ScStyleSheet* GetStyle( SCROW nRow ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const;
    const ScStyleSheet* GetAreaStyle( BOOL& rFound, SCROW nRow1, SCROW nRow2 ) const;

    void        FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset );
    BOOL        IsStyleSheetUsed( const ScStyleSheet& rStyle, BOOL bGatherAllStyles ) const;

                /// May return -1 if not found
    SCsROW      SearchStyle( SCsROW nRow, const ScStyleSheet* pSearchStyle,
                                BOOL bUp, BOOL bInSelection, const ScMarkData& rMark );
    BOOL        SearchStyleRange( SCsROW& rRow, SCsROW& rEndRow, const ScStyleSheet* pSearchStyle,
                                    BOOL bUp, BOOL bInSelection, const ScMarkData& rMark );

    BOOL        ApplyFlags( SCROW nStartRow, SCROW nEndRow, INT16 nFlags );
    BOOL        RemoveFlags( SCROW nStartRow, SCROW nEndRow, INT16 nFlags );
    void        ClearItems( SCROW nStartRow, SCROW nEndRow, const USHORT* pWhich );

    void        RemoveProtected( SCROW nStartRow, SCROW nEndRow );

    SCsROW      ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark, ScEditDataArray* pDataArray = NULL );
    void        DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark );

    void        ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark );
    void        ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark );

    long        GetNeededSize( SCROW nRow, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, const ScNeededSizeOptions& rOptions );
    USHORT      GetOptimalColWidth( OutputDevice* pDev, double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bFormula, USHORT nOldWidth,
                                    const ScMarkData* pMarkData,
                                    const ScColWidthParam* pParam );
    void        GetOptimalHeight( SCROW nStartRow, SCROW nEndRow, USHORT* pHeight,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bShrink, USHORT nMinHeight, SCROW nMinStart );
private:
    long        GetSimpleTextNeededSize( SCSIZE nIndex, OutputDevice* pDev,
                                    BOOL bWidth );
public:

                /// Including current, may return -1
    SCsROW      GetNextUnprotected( SCROW nRow, BOOL bUp ) const;

    void        GetFilterEntries(SCROW nStartRow, SCROW nEndRow, TypedScStrCollection& rStrings, bool& rHasDates);
    BOOL        GetDataEntries(SCROW nRow, TypedScStrCollection& rStrings, BOOL bLimit);

    void        UpdateInsertTabAbs(SCTAB nNewPos);
    BOOL        TestTabRefAbs(SCTAB nTable);
    BOOL        GetNextSpellingCell(SCROW& nRow, BOOL bInSel, const ScMarkData& rData) const;

    void        RemoveAutoSpellObj();

    void        StartListening( SvtListener& rLst, SCROW nRow );
    void        EndListening( SvtListener& rLst, SCROW nRow );
    void        MoveListeners( SvtBroadcaster& rSource, SCROW nDestRow );
    void        StartAllListeners();
    void        StartNeededListeners(); // only for cells where NeedsListening()==TRUE
    void        SetRelNameDirty();

    void        CompileDBFormula();
    void        CompileDBFormula( BOOL bCreateFormulaString );
    void        CompileNameFormula( BOOL bCreateFormulaString );
    void        CompileColRowNameFormula();

    sal_Int32   GetMaxStringLen( SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const;
    xub_StrLen  GetMaxNumberStringLen( sal_uInt16& nPrecision,
                                       SCROW nRowStart, SCROW nRowEnd ) const;

private:
    ScBaseCell* CloneCell(SCSIZE nIndex, USHORT nFlags, ScDocument& rDestDoc, const ScAddress& rDestPos);
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

    BOOL        Next( SCROW& rRow, ScBaseCell*& rpCell );
    SCSIZE      GetIndex() const;
};


class ScMarkedDataIter                  // Daten in selektierten Bereichen durchgehen
{
    const ScColumn*     pColumn;
    SCSIZE              nPos;
    ScMarkArrayIter*    pMarkIter;
    SCROW               nTop;
    SCROW               nBottom;
    BOOL                bNext;
    BOOL                bAll;

public:
                ScMarkedDataIter( const ScColumn* pCol, const ScMarkData* pMarkData,
                                    BOOL bAllIfNone = FALSE );
                ~ScMarkedDataIter();

    BOOL        Next( SCSIZE& rIndex );
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
