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

#ifndef SC_TABLE_HXX
#define SC_TABLE_HXX

#include <vector>
#include <memory>
#include <utility>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "column.hxx"
#include "sortparam.hxx"
#include "compressedarray.hxx"

#include <memory>
#include <set>
#include <boost/shared_ptr.hpp>

namespace utl {
    class SearchParam;
    class TextSearch;
}

namespace com { namespace sun { namespace star {
    namespace sheet {
        struct TablePageBreakData;
    }
} } }

class SfxItemSet;
class SfxStyleSheetBase;
class SvxBoxInfoItem;
class SvxBoxItem;
class SvxSearchItem;

class ScAutoFormat;
class ScAutoFormatData;
class ScBaseCell;
class ScDocument;
class ScDrawLayer;
class ScEditDataArray;
class ScFormulaCell;
class ScOutlineTable;
class ScPostIt;
class ScPrintSaverTab;
class ScProgress;
class ScProgress;
class ScRangeList;
class ScSheetEvents;
class ScSortInfoArray;
class ScStyleSheet;
class ScTableLink;
class ScTableProtection;
class ScUserListData;
struct RowInfo;
struct ScFunctionData;
struct ScLineFlags;
class CollatorWrapper;
class ScFlatUInt16RowSegments;
class ScFlatBoolRowSegments;
class ScFlatBoolColSegments;
struct ScSetStringParam;
struct ScColWidthParam;
struct ScColWidthParam;
class ScRangeName;

typedef boost::unordered_map< ::rtl::OUString, rtl::OUString, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > NameToNameMap;

class ScTable
{
private:
    typedef ::std::vector< ScRange > ScRangeVec;
    typedef ::std::pair< SCCOL, SCROW > ScAddress2D;
    typedef ::std::vector< ScAddress2D > ScAddress2DVec;
    typedef ::std::auto_ptr< ScAddress2DVec > ScAddress2DVecPtr;

                                            //  Daten pro Tabelle   ------------------
    ScColumn        aCol[MAXCOLCOUNT];

    String          aName;
    String          aCodeName;
    String          aComment;
    BOOL            bScenario;
    BOOL            bLayoutRTL;
    BOOL            bLoadingRTL;

    String          aLinkDoc;
    String          aLinkFlt;
    String          aLinkOpt;
    String          aLinkTab;
    ULONG           nLinkRefreshDelay;
    BYTE            nLinkMode;

    // Seitenformatvorlage
    String          aPageStyle;
    BOOL            bPageSizeValid;
    Size            aPageSizeTwips;                 // Groesse der Druck-Seite
    SCCOL           nRepeatStartX;                  // Wiederholungszeilen/Spalten
    SCCOL           nRepeatEndX;                    // REPEAT_NONE, wenn nicht benutzt
    SCROW           nRepeatStartY;
    SCROW           nRepeatEndY;

    ::std::auto_ptr<ScTableProtection> pTabProtection;

    USHORT*         pColWidth;
    ::boost::shared_ptr<ScFlatUInt16RowSegments> mpRowHeights;

    BYTE*           pColFlags;
    ScBitMaskCompressedArray< SCROW, BYTE>*     pRowFlags;
    ::boost::shared_ptr<ScFlatBoolColSegments>  mpHiddenCols;
    ::boost::shared_ptr<ScFlatBoolRowSegments>  mpHiddenRows;
    ::boost::shared_ptr<ScFlatBoolColSegments>  mpFilteredCols;
    ::boost::shared_ptr<ScFlatBoolRowSegments>  mpFilteredRows;

    ::std::set<SCROW>                      maRowPageBreaks;
    ::std::set<SCROW>                      maRowManualBreaks;
    ::std::set<SCCOL>                      maColPageBreaks;
    ::std::set<SCCOL>                      maColManualBreaks;

    ScOutlineTable* pOutlineTable;

    ScSheetEvents*  pSheetEvents;

    SCCOL           nTableAreaX;
    SCROW           nTableAreaY;
    BOOL            bTableAreaValid;

                                            //  interne Verwaltung  ------------------
    BOOL            bVisible;
    BOOL            bStreamValid;
    BOOL            bPendingRowHeights;
    BOOL            bCalcNotification;

    SCTAB           nTab;
    USHORT          nRecalcLvl;             // Rekursionslevel Size-Recalc
    ScDocument*     pDocument;
    utl::SearchParam*   pSearchParam;
    utl::TextSearch*    pSearchText;

    mutable String  aUpperName;             // #i62977# filled only on demand, reset in SetName

    ScAddress2DVecPtr mxUninitNotes;

    // SortierParameter um den Stackbedarf von Quicksort zu Minimieren
    ScSortParam     aSortParam;
    CollatorWrapper*    pSortCollator;
    BOOL            bGlobalKeepQuery;
    BOOL            bSharedNameInserted;

    ScRangeVec      aPrintRanges;
    bool            bPrintEntireSheet;

    ScRange*        pRepeatColRange;
    ScRange*        pRepeatRowRange;

    USHORT          nLockCount;

    ScRangeList*    pScenarioRanges;
    Color           aScenarioColor;
    Color           aTabBgColor;
    USHORT          nScenarioFlags;
    BOOL            bActiveScenario;
    NameToNameMap       localNameToGlobalName;
    mutable ScRangeName* mpRangeName;
    bool            mbPageBreaksValid;

friend class ScDocument;                    // fuer FillInfo
friend class ScDocumentIterator;
friend class ScValueIterator;
friend class ScDBQueryDataIterator;
friend class ScCellIterator;
friend class ScQueryCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScDocAttrIterator;
friend class ScAttrRectIterator;


public:
                ScTable( ScDocument* pDoc, SCTAB nNewTab, const String& rNewName,
                            BOOL bColInfo = TRUE, BOOL bRowInfo = TRUE );
                ~ScTable();

    ScOutlineTable* GetOutlineTable()               { return pOutlineTable; }

    SCSIZE      GetCellCount(SCCOL nCol) const;
    ULONG       GetCellCount() const;
    ULONG       GetWeightedCount() const;
    ULONG       GetCodeCount() const;       // RPN-Code in Formeln

    BOOL        SetOutlineTable( const ScOutlineTable* pNewOutline );
    void        StartOutlineTable();

    void        DoAutoOutline( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );

    BOOL        TestRemoveSubTotals( const ScSubTotalParam& rParam );
    void        RemoveSubTotals( ScSubTotalParam& rParam );
    BOOL        DoSubTotals( ScSubTotalParam& rParam );

    const ScSheetEvents* GetSheetEvents() const              { return pSheetEvents; }
    void        SetSheetEvents( const ScSheetEvents* pNew );

    BOOL        IsVisible() const                            { return bVisible; }
    void        SetVisible( BOOL bVis );

    BOOL        IsStreamValid() const                        { return bStreamValid; }
    void        SetStreamValid( BOOL bSet, BOOL bIgnoreLock = FALSE );

    BOOL        IsPendingRowHeights() const                  { return bPendingRowHeights; }
    void        SetPendingRowHeights( BOOL bSet );

    BOOL        GetCalcNotification() const                  { return bCalcNotification; }
    void        SetCalcNotification( BOOL bSet );

    BOOL        IsLayoutRTL() const                          { return bLayoutRTL; }
    BOOL        IsLoadingRTL() const                         { return bLoadingRTL; }
    void        SetLayoutRTL( BOOL bSet );
    void        SetLoadingRTL( BOOL bSet );

    BOOL        IsScenario() const                           { return bScenario; }
    void        SetScenario( BOOL bFlag );
    void        GetScenarioComment( String& rComment) const  { rComment = aComment; }
    void        SetScenarioComment( const String& rComment ) { aComment = rComment; }
    const Color& GetScenarioColor() const                    { return aScenarioColor; }
    void        SetScenarioColor(const Color& rNew)          { aScenarioColor = rNew; }
    const Color& GetTabBgColor() const;
    void         SetTabBgColor(const Color& rColor);
    USHORT      GetScenarioFlags() const                     { return nScenarioFlags; }
    void        SetScenarioFlags(USHORT nNew)                { nScenarioFlags = nNew; }
    void        SetActiveScenario(BOOL bSet)                 { bActiveScenario = bSet; }
    BOOL        IsActiveScenario() const                     { return bActiveScenario; }

    BYTE        GetLinkMode() const                         { return nLinkMode; }
    BOOL        IsLinked() const                            { return nLinkMode != SC_LINK_NONE; }
    const String& GetLinkDoc() const                        { return aLinkDoc; }
    const String& GetLinkFlt() const                        { return aLinkFlt; }
    const String& GetLinkOpt() const                        { return aLinkOpt; }
    const String& GetLinkTab() const                        { return aLinkTab; }
    ULONG       GetLinkRefreshDelay() const                 { return nLinkRefreshDelay; }

    void        SetLink( BYTE nMode, const String& rDoc, const String& rFlt,
                        const String& rOpt, const String& rTab, ULONG nRefreshDelay );

    void        GetName( String& rName ) const;
    void        SetName( const String& rNewName );

    void        GetCodeName( String& rName ) const {  rName = aCodeName; }
    void        SetCodeName( const String& rNewName ) { aCodeName = rNewName; }

    const String&   GetUpperName() const;

    const String&   GetPageStyle() const                    { return aPageStyle; }
    void            SetPageStyle( const String& rName );
    void            PageStyleModified( const String& rNewName );

    BOOL            IsProtected() const;
    void            SetProtection(const ScTableProtection* pProtect);
    ScTableProtection* GetProtection();

    Size            GetPageSize() const;
    void            SetPageSize( const Size& rSize );
    void            SetRepeatArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow );

    void        RemoveAutoSpellObj();

    void        LockTable();
    void        UnlockTable();

    BOOL        IsBlockEditable( SCCOL nCol1, SCROW nRow1, SCCOL nCol2,
                        SCROW nRow2, BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL        IsSelectionEditable( const ScMarkData& rMark,
                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;

    BOOL        HasBlockMatrixFragment( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;
    bool        HasSelectionMatrixFragment( const ScMarkData& rMark ) const;

    BOOL        IsBlockEmpty( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bIgnoreNotes = false ) const;

    void        PutCell( const ScAddress&, ScBaseCell* pCell );
    void        PutCell( SCCOL nCol, SCROW nRow, ScBaseCell* pCell );
    void        PutCell(SCCOL nCol, SCROW nRow, ULONG nFormatIndex, ScBaseCell* pCell);
                //  TRUE = Zahlformat gesetzt
    BOOL        SetString( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString,
                           ScSetStringParam* pParam = NULL );
    void        SetValue( SCCOL nCol, SCROW nRow, const double& rVal );
    void        SetError( SCCOL nCol, SCROW nRow, USHORT nError);

    void        GetString( SCCOL nCol, SCROW nRow, String& rString );
    void        GetInputString( SCCOL nCol, SCROW nRow, String& rString );
    double      GetValue( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetValue( rPos.Row() ) :
                            0.0;
                    }
    double      GetValue( SCCOL nCol, SCROW nRow );
    void        GetFormula( SCCOL nCol, SCROW nRow, String& rFormula );

    CellType    GetCellType( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetCellType( rPos.Row() ) :
                            CELLTYPE_NONE;
                    }
    CellType    GetCellType( SCCOL nCol, SCROW nRow ) const;
    ScBaseCell* GetCell( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetCell( rPos.Row() ) :
                            NULL;
                    }
    ScBaseCell* GetCell( SCCOL nCol, SCROW nRow ) const;

    void        GetFirstDataPos(SCCOL& rCol, SCROW& rRow) const;
    void        GetLastDataPos(SCCOL& rCol, SCROW& rRow) const;

    /** Returns the pointer to a cell note object at the passed cell address. */
    ScPostIt*   GetNote( SCCOL nCol, SCROW nRow );
    /** Sets the passed cell note object at the passed cell address. Takes ownership! */
    void        TakeNote( SCCOL nCol, SCROW nRow, ScPostIt*& rpNote );
    /** Returns and forgets the cell note object at the passed cell address. */
    ScPostIt*   ReleaseNote( SCCOL nCol, SCROW nRow );
    /** Deletes the note at the passed cell address. */
    void        DeleteNote( SCCOL nCol, SCROW nRow );
    /** Creates the captions of all uninitialized cell notes.
        @param bForced  True = always create all captions, false = skip when Undo is disabled. */
    void        InitializeNoteCaptions( bool bForced = false );

    BOOL        TestInsertRow( SCCOL nStartCol, SCCOL nEndCol, SCSIZE nSize );
    void        InsertRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize );
    void        DeleteRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize,
                            BOOL* pUndoOutline = NULL );

    BOOL        TestInsertCol( SCROW nStartRow, SCROW nEndRow, SCSIZE nSize );
    void        InsertCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize );
    void        DeleteCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize,
                            BOOL* pUndoOutline = NULL );

    void        DeleteArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, USHORT nDelFlag);
    void        CopyToClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScTable* pTable,
                            BOOL bKeepScenarioFlags, BOOL bCloneNoteCaptions);
    void        CopyToClip(const ScRangeList& rRanges, ScTable* pTable,
                           bool bKeepScenarioFlags, bool bCloneNoteCaptions);
    void        CopyFromClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, SCsCOL nDx, SCsROW nDy,
                                USHORT nInsFlag, BOOL bAsLink, BOOL bSkipAttrForEmpty, ScTable* pTable);
    void        StartListeningInArea( SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2 );
    void        BroadcastInArea( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2 );

    void        CopyToTable(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            USHORT nFlags, BOOL bMarked, ScTable* pDestTab,
                            const ScMarkData* pMarkData = NULL,
                            BOOL bAsLink = FALSE, BOOL bColRowFlags = TRUE);
    void        UndoToTable(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            USHORT nFlags, BOOL bMarked, ScTable* pDestTab,
                            const ScMarkData* pMarkData = NULL);

    void        TransposeClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                ScTable* pTransClip, USHORT nFlags, BOOL bAsLink );

                //  Markierung von diesem Dokument
    void        MixMarked( const ScMarkData& rMark, USHORT nFunction,
                            BOOL bSkipEmpty, ScTable* pSrcTab );
    void        MixData( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            USHORT nFunction, BOOL bSkipEmpty, ScTable* pSrcTab );

    void        CopyData( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            SCCOL nDestCol, SCROW nDestRow, SCTAB nDestTab );

    void        CopyScenarioFrom( const ScTable* pSrcTab );
    void        CopyScenarioTo( ScTable* pDestTab ) const;
    BOOL        TestCopyScenarioTo( const ScTable* pDestTab ) const;
    void        MarkScenarioIn( ScMarkData& rMark, USHORT nNeededBits ) const;
    BOOL        HasScenarioRange( const ScRange& rRange ) const;
    void        InvalidateScenarioRanges();
    const ScRangeList* GetScenarioRanges() const;

    void        CopyUpdated( const ScTable* pPosTab, ScTable* pDestTab ) const;

    void        InvalidateTableArea();
    void        InvalidatePageBreaks();

    BOOL        GetCellArea( SCCOL& rEndCol, SCROW& rEndRow ) const;            // FALSE = leer
    BOOL        GetTableArea( SCCOL& rEndCol, SCROW& rEndRow ) const;
    BOOL        GetPrintArea( SCCOL& rEndCol, SCROW& rEndRow, BOOL bNotes ) const;
    BOOL        GetPrintAreaHor( SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rEndCol, BOOL bNotes ) const;
    BOOL        GetPrintAreaVer( SCCOL nStartCol, SCCOL nEndCol,
                                SCROW& rEndRow, BOOL bNotes ) const;

    BOOL        GetDataStart( SCCOL& rStartCol, SCROW& rStartRow ) const;

    void        ExtendPrintArea( OutputDevice* pDev,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL& rEndCol, SCROW nEndRow );

    void        GetDataArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow,
                             BOOL bIncludeOld, bool bOnlyDown ) const;

    bool        ShrinkToUsedDataArea( bool& o_bShrunk, SCCOL& rStartCol, SCROW& rStartRow,
                                      SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly ) const;

    SCSIZE      GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, ScDirection eDir );

    void        FindAreaPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY );
    void        GetNextPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark );

    void        LimitChartArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow );

    BOOL        HasData( SCCOL nCol, SCROW nRow );
    BOOL        HasStringData( SCCOL nCol, SCROW nRow );
    BOOL        HasValueData( SCCOL nCol, SCROW nRow );
    BOOL        HasStringCells( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow ) const;

    USHORT      GetErrCode( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetErrCode( rPos.Row() ) :
                            0;
                    }

    void        ResetChanged( const ScRange& rRange );

    void        SetDirty();
    void        SetDirty( const ScRange& );
    void        SetDirtyAfterLoad();
    void        SetDirtyVar();
    void        SetTableOpDirty( const ScRange& );
    void        CalcAll();
    void        CalcAfterLoad();
    void        CompileAll();
    void        CompileXML( ScProgress& rProgress );

    void        UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                    ScDocument* pUndoDoc = NULL, BOOL bIncludeDraw = TRUE, bool bUpdateNoteCaptionPos = true );

    void        UpdateDrawRef( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz, bool bUpdateNoteCaptionPos = true );

    void        UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc );

    void        UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void        UpdateInsertTab(SCTAB nTable);
    void        UpdateDeleteTab( SCTAB nTable, BOOL bIsMove, ScTable* pRefUndo = NULL );
    void        UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo, ScProgress& );
    void        UpdateCompile( BOOL bForceIfNameInUse = FALSE );
    void        SetTabNo(SCTAB nNewTab);
    BOOL        IsRangeNameInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                 USHORT nIndex) const;
    void        FindRangeNamesInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                 std::set<USHORT>& rIndexes) const;
    void        ReplaceRangeNamesInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                      const ScRangeData::IndexMap& rMap );
    void        Fill( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        ULONG nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                        double nStepValue, double nMaxValue);
    String      GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY );

    void        UpdateSelectionFunction( ScFunctionData& rData,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark );

    void        AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    USHORT nFormatNo );
    void        GetAutoFormatData(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, ScAutoFormatData& rData);
    void        ScReplaceTabsStr( String& rStr, const String& rSrch, const String& rRepl ); // aus sw
    BOOL        SearchAndReplace(const SvxSearchItem& rSearchItem,
                                SCCOL& rCol, SCROW& rRow, ScMarkData& rMark,
                                String& rUndoStr, ScDocument* pUndoDoc);

    void        FindMaxRotCol( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2 );

    void        GetBorderLines( SCCOL nCol, SCROW nRow,
                                const SvxBorderLine** ppLeft, const SvxBorderLine** ppTop,
                                const SvxBorderLine** ppRight, const SvxBorderLine** ppBottom ) const;

    bool        HasAttrib( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, USHORT nMask ) const;
    bool        HasAttribSelection( const ScMarkData& rMark, USHORT nMask ) const;
    bool        ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow,
                                BOOL bRefresh, BOOL bAttrs );
    const SfxPoolItem*      GetAttr( SCCOL nCol, SCROW nRow, USHORT nWhich ) const;
    const ScPatternAttr*    GetPattern( SCCOL nCol, SCROW nRow ) const;
    const ScPatternAttr*    GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const;

    ULONG                   GetNumberFormat( const ScAddress& rPos ) const
                                {
                                    return ValidColRow(rPos.Col(),rPos.Row()) ?
                                        aCol[rPos.Col()].GetNumberFormat( rPos.Row() ) :
                                        0;
                                }
    ULONG                   GetNumberFormat( SCCOL nCol, SCROW nRow ) const;
    sal_uInt32              GetNumberFormat( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const;
    void                    MergeSelectionPattern( ScMergePatternState& rState,
                                                const ScMarkData& rMark, BOOL bDeep ) const;
    void                    MergePatternArea( ScMergePatternState& rState, SCCOL nCol1, SCROW nRow1,
                                                SCCOL nCol2, SCROW nRow2, BOOL bDeep ) const;
    void                    MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                                            ScLineFlags& rFlags,
                                            SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow ) const;
    void                    ApplyBlockFrame( const SvxBoxItem* pLineOuter,
                                            const SvxBoxInfoItem* pLineInner,
                                            SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow );

    void        ApplyAttr( SCCOL nCol, SCROW nRow, const SfxPoolItem& rAttr );
    void        ApplyPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr );
    void        ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                  const ScPatternAttr& rAttr, ScEditDataArray* pDataArray = NULL );
    bool        SetAttrEntries(SCCOL nCol, ScAttrEntry* pData, SCSIZE nSize);

    void        SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr, BOOL bPutToPool = FALSE )
                    {
                        if (ValidColRow(rPos.Col(),rPos.Row()))
                            aCol[rPos.Col()].SetPattern( rPos.Row(), rAttr, bPutToPool );
                    }
    void        SetPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr, BOOL bPutToPool = FALSE );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, short nNewType );

    void        ApplyStyle( SCCOL nCol, SCROW nRow, const ScStyleSheet& rStyle );
    void        ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, const ScStyleSheet& rStyle );
    void        ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);
    void        ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const SvxBorderLine* pLine, BOOL bColorOnly );

    const ScStyleSheet* GetStyle( SCCOL nCol, SCROW nRow ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const;
    const ScStyleSheet* GetAreaStyle( BOOL& rFound, SCCOL nCol1, SCROW nRow1,
                                                    SCCOL nCol2, SCROW nRow2 ) const;

    void        StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY );

    BOOL        IsStyleSheetUsed( const ScStyleSheet& rStyle, BOOL bGatherAllStyles ) const;

    bool        ApplyFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, INT16 nFlags );
    bool        RemoveFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, INT16 nFlags );

    void        ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark, ScEditDataArray* pDataArray = NULL );
    void        DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark );

    void        ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark );
    void        ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark );

    const ScRange*  GetRepeatColRange() const   { return pRepeatColRange; }
    const ScRange*  GetRepeatRowRange() const   { return pRepeatRowRange; }
    void            SetRepeatColRange( const ScRange* pNew );
    void            SetRepeatRowRange( const ScRange* pNew );

    USHORT          GetPrintRangeCount() const          { return static_cast< USHORT >( aPrintRanges.size() ); }
    const ScRange*  GetPrintRange(USHORT nPos) const;
    /** Returns true, if the sheet is always printed. */
    bool            IsPrintEntireSheet() const          { return bPrintEntireSheet; }

    /** Removes all print ranges. */
    void            ClearPrintRanges();
    /** Adds a new print ranges. */
    void            AddPrintRange( const ScRange& rNew );
    /** Marks the specified sheet to be printed completely. Deletes old print ranges! */
    void            SetPrintEntireSheet();

    void            FillPrintSaver( ScPrintSaverTab& rSaveTab ) const;
    void            RestorePrintRanges( const ScPrintSaverTab& rSaveTab );

    USHORT      GetOptimalColWidth( SCCOL nCol, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bFormula, const ScMarkData* pMarkData,
                                    const ScColWidthParam* pParam );
    BOOL        SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, USHORT nExtra,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bForce,
                                    ScProgress* pOuterProgress = NULL, ULONG nProgressStart = 0 );

    void        SetOptimalHeightOnly(SCROW nStartRow, SCROW nEndRow, USHORT nExtra,
                                     OutputDevice* pDev,
                                     double nPPTX, double nPPTY,
                                     const Fraction& rZoomX, const Fraction& rZoomY,
                                     BOOL bForce,
                                     ScProgress* pOuterProgress = NULL, ULONG nProgressStart = 0 );

    long        GetNeededSize( SCCOL nCol, SCROW nRow,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, BOOL bTotalSize );
    void        SetColWidth( SCCOL nCol, USHORT nNewWidth );
    void        SetColWidthOnly( SCCOL nCol, USHORT nNewWidth );
    void        SetRowHeight( SCROW nRow, USHORT nNewHeight );
    BOOL        SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, USHORT nNewHeight,
                                    double nPPTX, double nPPTY );

    /**
     * Set specified row height to specified ranges.  Don't check for drawing
     * objects etc.  Just set the row height.  Nothing else.
     *
     * Note that setting a new row height via this function will not
     * invalidate page breaks.
     */
    void        SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, USHORT nNewHeight );

                        // nPPT fuer Test auf Veraenderung
    void        SetManualHeight( SCROW nStartRow, SCROW nEndRow, BOOL bManual );

    USHORT      GetColWidth( SCCOL nCol ) const;
    SC_DLLPUBLIC USHORT GetRowHeight( SCROW nRow, SCROW* pStartRow = NULL, SCROW* pEndRow = NULL, bool bHiddenAsZero = true ) const;
    ULONG       GetRowHeight( SCROW nStartRow, SCROW nEndRow ) const;
    ULONG       GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow, double fScale ) const;
    ULONG       GetColOffset( SCCOL nCol ) const;
    ULONG       GetRowOffset( SCROW nRow ) const;

    /**
     * Get the last row such that the height of row 0 to the end row is as
     * high as possible without exceeding the specified height value.
     *
     * @param nHeight maximum desired height
     *
     * @return SCROW last row of the range within specified height.
     */
    SCROW       GetRowForHeight(ULONG nHeight) const;

    USHORT      GetOriginalWidth( SCCOL nCol ) const;
    USHORT      GetOriginalHeight( SCROW nRow ) const;

    USHORT      GetCommonWidth( SCCOL nEndCol );

    SCROW       GetHiddenRowCount( SCROW nRow );

    void        ShowCol(SCCOL nCol, bool bShow);
    void        ShowRow(SCROW nRow, bool bShow);
    void        DBShowRow(SCROW nRow, bool bShow);

    void        ShowRows(SCROW nRow1, SCROW nRow2, bool bShow);
    void        DBShowRows(SCROW nRow1, SCROW nRow2, bool bShow);

    void        SetColFlags( SCCOL nCol, BYTE nNewFlags );
    void        SetRowFlags( SCROW nRow, BYTE nNewFlags );
    void        SetRowFlags( SCROW nStartRow, SCROW nEndRow, BYTE nNewFlags );

                /// @return  the index of the last row with any set flags (auto-pagebreak is ignored).
    SCROW      GetLastFlaggedRow() const;

                /// @return  the index of the last changed column (flags and column width, auto pagebreak is ignored).
    SCCOL      GetLastChangedCol() const;
                /// @return  the index of the last changed row (flags and row height, auto pagebreak is ignored).
    SCROW      GetLastChangedRow() const;

    BYTE        GetColFlags( SCCOL nCol ) const;
    BYTE        GetRowFlags( SCROW nRow ) const;

    const ScBitMaskCompressedArray< SCROW, BYTE> * GetRowFlagsArray() const
                    { return pRowFlags; }

    BOOL        UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, BOOL bShow );
    BOOL        UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, BOOL bShow );

    void        UpdatePageBreaks( const ScRange* pUserArea );
    void        RemoveManualBreaks();
    BOOL        HasManualBreaks() const;
    void        SetRowManualBreaks( const ::std::set<SCROW>& rBreaks );
    void        SetColManualBreaks( const ::std::set<SCCOL>& rBreaks );

    void        GetAllRowBreaks(::std::set<SCROW>& rBreaks, bool bPage, bool bManual) const;
    void        GetAllColBreaks(::std::set<SCCOL>& rBreaks, bool bPage, bool bManual) const;
    bool        HasRowPageBreak(SCROW nRow) const;
    bool        HasColPageBreak(SCCOL nCol) const;
    bool        HasRowManualBreak(SCROW nRow) const;
    bool        HasColManualBreak(SCCOL nCol) const;

    /**
     * Get the row position of the next manual break that occurs at or below
     * specified row.  When no more manual breaks are present at or below
     * the specified row, -1 is returned.
     *
     * @param nRow row at which the search begins.
     *
     * @return SCROW next row position with manual page break, or -1 if no
     *         more manual breaks are present.
     */
    SCROW       GetNextManualBreak(SCROW nRow) const;

    void        RemoveRowPageBreaks(SCROW nStartRow, SCROW nEndRow);
    void        RemoveRowBreak(SCROW nRow, bool bPage, bool bManual);
    void        RemoveColBreak(SCCOL nCol, bool bPage, bool bManual);
    void        SetRowBreak(SCROW nRow, bool bPage, bool bManual);
    void        SetColBreak(SCCOL nCol, bool bPage, bool bManual);
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::sheet::TablePageBreakData> GetRowBreakData() const;

    bool        RowHidden(SCROW nRow, SCROW* pFirstRow = NULL, SCROW* pLastRow = NULL) const;
    bool        RowHiddenLeaf(SCROW nRow, SCROW* pFirstRow = NULL, SCROW* pLastRow = NULL) const;
    bool        HasHiddenRows(SCROW nStartRow, SCROW nEndRow) const;
    bool        ColHidden(SCCOL nCol, SCCOL* pFirstCol = NULL, SCCOL* pLastCol = NULL) const;
    bool        SetRowHidden(SCROW nStartRow, SCROW nEndRow, bool bHidden);
    bool        SetColHidden(SCCOL nStartCol, SCCOL nEndCol, bool bHidden);
    void        CopyColHidden(ScTable& rTable, SCCOL nStartCol, SCCOL nEndCol);
    void        CopyRowHidden(ScTable& rTable, SCROW nStartRow, SCROW nEndRow);
    void        CopyRowHeight(ScTable& rSrcTable, SCROW nStartRow, SCROW nEndRow, SCROW nSrcOffset);
    SCROW       FirstVisibleRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW       LastVisibleRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW       CountVisibleRows(SCROW nStartRow, SCROW nEndRow) const;
    sal_uInt32  GetTotalRowHeight(SCROW nStartRow, SCROW nEndRow) const;

    SCCOLROW    LastHiddenColRow(SCCOLROW nPos, bool bCol) const;

    bool        RowFiltered(SCROW nRow, SCROW* pFirstRow = NULL, SCROW* pLastRow = NULL) const;
    bool        ColFiltered(SCCOL nCol, SCCOL* pFirstCol = NULL, SCCOL* pLastCol = NULL) const;
    bool        HasFilteredRows(SCROW nStartRow, SCROW nEndRow) const;
    void        CopyColFiltered(ScTable& rTable, SCCOL nStartCol, SCCOL nEndCol);
    void        CopyRowFiltered(ScTable& rTable, SCROW nStartRow, SCROW nEndRow);
    void        SetRowFiltered(SCROW nStartRow, SCROW nEndRow, bool bFiltered);
    void        SetColFiltered(SCCOL nStartCol, SCCOL nEndCol, bool bFiltered);
    SCROW       FirstNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW       LastNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW       CountNonFilteredRows(SCROW nStartRow, SCROW nEndRow) const;

    void        SyncColRowFlags();

    void        StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );
    void        ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );

    void        Sort(const ScSortParam& rSortParam, BOOL bKeepQuery);
    BOOL        ValidQuery(SCROW nRow, const ScQueryParam& rQueryParam,
                    BOOL* pSpecial = NULL, ScBaseCell* pCell = NULL,
                    BOOL* pbTestEqualCondition = NULL );
    void        TopTenQuery( ScQueryParam& );
    SCSIZE      Query(ScQueryParam& rQueryParam, BOOL bKeepSub);
    BOOL        CreateQueryParam(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);

    void        GetFilterEntries(SCCOL nCol, SCROW nRow1, SCROW nRow2, TypedScStrCollection& rStrings, bool& rHasDates);
    void        GetFilteredFilterEntries( SCCOL nCol, SCROW nRow1, SCROW nRow2, const ScQueryParam& rParam, TypedScStrCollection& rStrings, bool& rHasDates );
    BOOL        GetDataEntries(SCCOL nCol, SCROW nRow, TypedScStrCollection& rStrings, BOOL bLimit);

    BOOL        HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );
    BOOL        HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );

    void        DoColResize( SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd );


    sal_Int32   GetMaxStringLen( SCCOL nCol,
                                    SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const;
    xub_StrLen  GetMaxNumberStringLen( sal_uInt16& nPrecision,
                                       SCCOL nCol,
                                       SCROW nRowStart, SCROW nRowEnd ) const;

    void        FindConditionalFormat( ULONG nKey, ScRangeList& rRanges );

    void        IncRecalcLevel();
    void        DecRecalcLevel( bool bUpdateNoteCaptionPos = true );

    BOOL        IsSortCollatorGlobal() const;
    void        InitSortCollator( const ScSortParam& rPar );
    void        DestroySortCollator();
    void        SetDrawPageSize( bool bResetStreamValid = true, bool bUpdateNoteCaptionPos = true );

    ScRangeName* GetRangeName() const;

private:
    void        FillSeries( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                ULONG nFillCount, FillDir eFillDir, FillCmd eFillCmd,
                                FillDateCmd eFillDateCmd,
                                double nStepValue, double nMaxValue, USHORT nMinDigits,
                                BOOL bAttribs, ScProgress& rProgress );
    void        FillAnalyse( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                FillCmd& rCmd, FillDateCmd& rDateCmd,
                                double& rInc, USHORT& rMinDigits,
                                ScUserListData*& rListData, USHORT& rListIndex);
    void        FillAuto( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        ULONG nFillCount, FillDir eFillDir, ScProgress& rProgress );

    BOOL        ValidNextPos( SCCOL nCol, SCROW nRow, const ScMarkData& rMark,
                                BOOL bMarked, BOOL bUnprotected );

    void        AutoFormatArea(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                const ScPatternAttr& rAttr, USHORT nFormatNo);
    void        GetAutoFormatAttr(SCCOL nCol, SCROW nRow, USHORT nIndex, ScAutoFormatData& rData);
    void        GetAutoFormatFrame(SCCOL nCol, SCROW nRow, USHORT nFlags, USHORT nIndex, ScAutoFormatData& rData);
    BOOL        SearchCell(const SvxSearchItem& rSearchItem, SCCOL nCol, SCROW nRow,
                            const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc);
    BOOL        Search(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc);
    BOOL        SearchAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                        String& rUndoStr, ScDocument* pUndoDoc);
    BOOL        Replace(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc);
    BOOL        ReplaceAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                            String& rUndoStr, ScDocument* pUndoDoc);

    BOOL        SearchStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                                ScMarkData& rMark);
    BOOL        ReplaceStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                                ScMarkData& rMark, BOOL bIsUndo);
    BOOL        SearchAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark);
    BOOL        ReplaceAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                                ScDocument* pUndoDoc);
    bool        SearchAndReplaceEmptyCells(
                    const SvxSearchItem& rSearchItem,
                    SCCOL& rCol, SCROW& rRow, ScMarkData& rMark,
                    String& rUndoStr, ScDocument* pUndoDoc);
    bool        SearchRangeForEmptyCell(const ScRange& rRange,
                    const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                    String& rUndoStr, ScDocument* pUndoDoc);
    bool        SearchRangeForAllEmptyCells(const ScRange& rRange,
                    const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                    String& rUndoStr, ScDocument* pUndoDoc);

                                // benutzen globalen SortParam:
    BOOL        IsSorted(SCCOLROW nStart, SCCOLROW nEnd);
    void        DecoladeRow( ScSortInfoArray*, SCROW nRow1, SCROW nRow2 );
    void        SwapCol(SCCOL nCol1, SCCOL nCol2);
    void        SwapRow(SCROW nRow1, SCROW nRow2);
    short       CompareCell( USHORT nSort,
                    ScBaseCell* pCell1, SCCOL nCell1Col, SCROW nCell1Row,
                    ScBaseCell* pCell2, SCCOL nCell2Col, SCROW nCell2Row );
    short       Compare(SCCOLROW nIndex1, SCCOLROW nIndex2);
    short       Compare( ScSortInfoArray*, SCCOLROW nIndex1, SCCOLROW nIndex2);
    ScSortInfoArray*    CreateSortInfoArray( SCCOLROW nInd1, SCCOLROW nInd2 );
    void        QuickSort( ScSortInfoArray*, SCsCOLROW nLo, SCsCOLROW nHi);
    void        SortReorder( ScSortInfoArray*, ScProgress& );

    BOOL        CreateExcelQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);
    BOOL        CreateStarQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);
    void        GetUpperCellString(SCCOL nCol, SCROW nRow, String& rStr);

    BOOL        RefVisible(ScFormulaCell* pCell);

    BOOL        IsEmptyLine(SCROW nRow, SCCOL nStartCol, SCCOL nEndCol);

    void        IncDate(double& rVal, USHORT& nDayOfMonth, double nStep, FillDateCmd eCmd);
    void        FillFormula(ULONG& nFormulaCounter, BOOL bFirst, ScFormulaCell* pSrcCell,
                            SCCOL nDestCol, SCROW nDestRow, BOOL bLast );
    void        UpdateInsertTabAbs(SCTAB nNewPos);
    BOOL        GetNextSpellingCell(SCCOL& rCol, SCROW& rRow, BOOL bInSel,
                                    const ScMarkData& rMark) const;
    BOOL        GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark );
    BOOL        TestTabRefAbs(SCTAB nTable);
    void        CompileDBFormula();
    void        CompileDBFormula( BOOL bCreateFormulaString );
    void        CompileNameFormula( BOOL bCreateFormulaString );
    void        CompileColRowNameFormula();

    void        StartListening( const ScAddress& rAddress, SvtListener* pListener );
    void        EndListening( const ScAddress& rAddress, SvtListener* pListener );
    void        StartAllListeners();
    void        StartNeededListeners(); // only for cells where NeedsListening()==TRUE
    void        SetRelNameDirty();

    void        SetLoadingMedium(bool bLoading);

    SCSIZE      FillMaxRot( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2,
                            SCCOL nCol, SCROW nAttrRow1, SCROW nAttrRow2, SCSIZE nArrY,
                            const ScPatternAttr* pPattern, const SfxItemSet* pCondSet );

    // idle calculation of OutputDevice text width for cell
    // also invalidates script type, broadcasts for "calc as shown"
    void        InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo,
                                     BOOL bNumFormatChanged, BOOL bBroadcast );

    void        SkipFilteredRows(SCROW& rRow, SCROW& rLastNonFilteredRow, bool bForward);

    /**
     * In case the cell text goes beyond the column width, move the max column
     * position to the right.  This is called from ExtendPrintArea.
     */
    void        MaybeAddExtraColumn(SCCOL& rCol, SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY);

    /**
     * Use this to iterate through non-empty visible cells in a single column.
     */
    class VisibleDataCellIterator
    {
    public:
        static SCROW ROW_NOT_FOUND;

        explicit VisibleDataCellIterator(ScFlatBoolRowSegments& rRowSegs, ScColumn& rColumn);
        ~VisibleDataCellIterator();

        /**
         * Set the start row position.  In case there is not visible data cell
         * at the specified row position, it will move to the position of the
         * first visible data cell below that point.
         *
         * @return First visible data cell if found, or NULL otherwise.
         */
        ScBaseCell* reset(SCROW nRow);

        /**
         * Find the next visible data cell position.
         *
         * @return Next visible data cell if found, or NULL otherwise.
         */
        ScBaseCell* next();

        /**
         * Get the current row position.
         *
         * @return Current row position, or ROW_NOT_FOUND if the iterator
         *         doesn't point to a valid data cell position.
         */
        SCROW getRow() const;

    private:
        ScFlatBoolRowSegments& mrRowSegs;
        ScColumn& mrColumn;
        ScBaseCell* mpCell;
        SCROW mnCurRow;
        SCROW mnUBound;
    };
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
