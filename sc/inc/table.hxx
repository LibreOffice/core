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


struct ScShowRowsEntry
{
    SCROW   mnRow1;
    SCROW   mnRow2;
    bool    mbShow;

    ScShowRowsEntry( SCROW nR1, SCROW nR2, bool bS ) :
        mnRow1(nR1), mnRow2(nR2), mbShow(bS) {}
};


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
    sal_Bool            bScenario;
    sal_Bool            bLayoutRTL;
    sal_Bool            bLoadingRTL;

    String          aLinkDoc;
    String          aLinkFlt;
    String          aLinkOpt;
    String          aLinkTab;
    sal_uLong           nLinkRefreshDelay;
    sal_uInt8           nLinkMode;

    // Seitenformatvorlage
    String          aPageStyle;
    sal_Bool            bPageSizeValid;
    Size            aPageSizeTwips;                 // Groesse der Druck-Seite
    SCCOL           nRepeatStartX;                  // Wiederholungszeilen/Spalten
    SCCOL           nRepeatEndX;                    // REPEAT_NONE, wenn nicht benutzt
    SCROW           nRepeatStartY;
    SCROW           nRepeatEndY;

    ::std::auto_ptr<ScTableProtection> pTabProtection;

    sal_uInt16*         pColWidth;
    ::boost::shared_ptr<ScFlatUInt16RowSegments> mpRowHeights;

    sal_uInt8*          pColFlags;
    ScBitMaskCompressedArray< SCROW, sal_uInt8>*     pRowFlags;
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
    sal_Bool            bTableAreaValid;

                                            //  interne Verwaltung  ------------------
    sal_Bool            bVisible;
    sal_Bool            bStreamValid;
    sal_Bool            bPendingRowHeights;
    sal_Bool            bCalcNotification;

    SCTAB           nTab;
    sal_uInt16          nRecalcLvl;             // Rekursionslevel Size-Recalc
    ScDocument*     pDocument;
    utl::SearchParam*   pSearchParam;
    utl::TextSearch*    pSearchText;

    mutable String  aUpperName;             // #i62977# filled only on demand, reset in SetName

    ScAddress2DVecPtr mxUninitNotes;

    // SortierParameter um den Stackbedarf von Quicksort zu Minimieren
    ScSortParam     aSortParam;
    CollatorWrapper*    pSortCollator;
    sal_Bool            bGlobalKeepQuery;
    sal_Bool            bSharedNameInserted;

    ScRangeVec      aPrintRanges;
    sal_Bool            bPrintEntireSheet;

    ScRange*        pRepeatColRange;
    ScRange*        pRepeatRowRange;

    sal_uInt16          nLockCount;

    ScRangeList*    pScenarioRanges;
    Color           aScenarioColor;
    Color           aTabBgColor;
    sal_uInt16          nScenarioFlags;
    sal_Bool            bActiveScenario;
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
                            sal_Bool bColInfo = sal_True, sal_Bool bRowInfo = sal_True );
                ~ScTable();

    ScOutlineTable* GetOutlineTable()               { return pOutlineTable; }

    SCSIZE      GetCellCount(SCCOL nCol) const;
    sal_uLong       GetCellCount() const;
    sal_uLong       GetWeightedCount() const;
    sal_uLong       GetCodeCount() const;       // RPN-Code in Formeln

    sal_Bool        SetOutlineTable( const ScOutlineTable* pNewOutline );
    void        StartOutlineTable();

    void        DoAutoOutline( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );

    sal_Bool        TestRemoveSubTotals( const ScSubTotalParam& rParam );
    void        RemoveSubTotals( ScSubTotalParam& rParam );
    sal_Bool        DoSubTotals( ScSubTotalParam& rParam );

    const ScSheetEvents* GetSheetEvents() const              { return pSheetEvents; }
    void        SetSheetEvents( const ScSheetEvents* pNew );

    sal_Bool        IsVisible() const                            { return bVisible; }
    void        SetVisible( sal_Bool bVis );

    sal_Bool        IsStreamValid() const                        { return bStreamValid; }
    void        SetStreamValid( sal_Bool bSet, sal_Bool bIgnoreLock = sal_False );

    sal_Bool        IsPendingRowHeights() const                  { return bPendingRowHeights; }
    void        SetPendingRowHeights( sal_Bool bSet );

    sal_Bool        GetCalcNotification() const                  { return bCalcNotification; }
    void        SetCalcNotification( sal_Bool bSet );

    sal_Bool        IsLayoutRTL() const                          { return bLayoutRTL; }
    sal_Bool        IsLoadingRTL() const                         { return bLoadingRTL; }
    void        SetLayoutRTL( sal_Bool bSet );
    void        SetLoadingRTL( sal_Bool bSet );

    sal_Bool        IsScenario() const                           { return bScenario; }
    void        SetScenario( sal_Bool bFlag );
    void        GetScenarioComment( String& rComment) const  { rComment = aComment; }
    void        SetScenarioComment( const String& rComment ) { aComment = rComment; }
    const Color& GetScenarioColor() const                    { return aScenarioColor; }
    void        SetScenarioColor(const Color& rNew)          { aScenarioColor = rNew; }
    const Color& GetTabBgColor() const;
    void         SetTabBgColor(const Color& rColor);
    sal_uInt16      GetScenarioFlags() const                     { return nScenarioFlags; }
    void        SetScenarioFlags(sal_uInt16 nNew)                { nScenarioFlags = nNew; }
    void        SetActiveScenario(sal_Bool bSet)                 { bActiveScenario = bSet; }
    sal_Bool        IsActiveScenario() const                     { return bActiveScenario; }

    sal_uInt8       GetLinkMode() const                         { return nLinkMode; }
    sal_Bool        IsLinked() const                            { return nLinkMode != SC_LINK_NONE; }
    const String& GetLinkDoc() const                        { return aLinkDoc; }
    const String& GetLinkFlt() const                        { return aLinkFlt; }
    const String& GetLinkOpt() const                        { return aLinkOpt; }
    const String& GetLinkTab() const                        { return aLinkTab; }
    sal_uLong       GetLinkRefreshDelay() const                 { return nLinkRefreshDelay; }

    void        SetLink( sal_uInt8 nMode, const String& rDoc, const String& rFlt,
                        const String& rOpt, const String& rTab, sal_uLong nRefreshDelay );

    void        GetName( String& rName ) const;
    void        SetName( const String& rNewName );

    void        GetCodeName( String& rName ) const {  rName = aCodeName; }
    void        SetCodeName( const String& rNewName ) { aCodeName = rNewName; }

    const String&   GetUpperName() const;

    const String&   GetPageStyle() const                    { return aPageStyle; }
    void            SetPageStyle( const String& rName );
    void            PageStyleModified( const String& rNewName );

    sal_Bool            IsProtected() const;
    void            SetProtection(const ScTableProtection* pProtect);
    ScTableProtection* GetProtection();

    Size            GetPageSize() const;
    void            SetPageSize( const Size& rSize );
    void            SetRepeatArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow );

    void        RemoveAutoSpellObj();

    void        LockTable();
    void        UnlockTable();

    sal_Bool        IsBlockEditable( SCCOL nCol1, SCROW nRow1, SCCOL nCol2,
                        SCROW nRow2, sal_Bool* pOnlyNotBecauseOfMatrix = NULL ) const;
    sal_Bool        IsSelectionEditable( const ScMarkData& rMark,
                        sal_Bool* pOnlyNotBecauseOfMatrix = NULL ) const;

    sal_Bool        HasBlockMatrixFragment( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;
    sal_Bool        HasSelectionMatrixFragment( const ScMarkData& rMark ) const;

    sal_Bool        IsBlockEmpty( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bIgnoreNotes = false ) const;

    void        PutCell( const ScAddress&, ScBaseCell* pCell );
//UNUSED2009-05 void        PutCell( const ScAddress&, sal_uLong nFormatIndex, ScBaseCell* pCell);
    void        PutCell( SCCOL nCol, SCROW nRow, ScBaseCell* pCell );
    void        PutCell(SCCOL nCol, SCROW nRow, sal_uLong nFormatIndex, ScBaseCell* pCell);
                //  sal_True = Zahlformat gesetzt
    sal_Bool        SetString( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString,
                           SvNumberFormatter* pFormatter = NULL, bool bDetectNumberFormat = true );
    void        SetValue( SCCOL nCol, SCROW nRow, const double& rVal );
    void        SetError( SCCOL nCol, SCROW nRow, sal_uInt16 nError);

    void        GetString( SCCOL nCol, SCROW nRow, String& rString );
    void        GetInputString( SCCOL nCol, SCROW nRow, String& rString );
    double      GetValue( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetValue( rPos.Row() ) :
                            0.0;
                    }
    double      GetValue( SCCOL nCol, SCROW nRow );
    void        GetFormula( SCCOL nCol, SCROW nRow, String& rFormula,
                            sal_Bool bAsciiExport = sal_False );

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

    sal_Bool        TestInsertRow( SCCOL nStartCol, SCCOL nEndCol, SCSIZE nSize );
    void        InsertRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize );
    void        DeleteRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize,
                            sal_Bool* pUndoOutline = NULL );

    sal_Bool        TestInsertCol( SCROW nStartRow, SCROW nEndRow, SCSIZE nSize );
    void        InsertCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize );
    void        DeleteCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize,
                            sal_Bool* pUndoOutline = NULL );

    void        DeleteArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sal_uInt16 nDelFlag);
    void        CopyToClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScTable* pTable,
                            sal_Bool bKeepScenarioFlags, sal_Bool bCloneNoteCaptions);
    void        CopyToClip(const ScRangeList& rRanges, ScTable* pTable,
                           bool bKeepScenarioFlags, bool bCloneNoteCaptions);
    void        CopyFromClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, SCsCOL nDx, SCsROW nDy,
                                sal_uInt16 nInsFlag, sal_Bool bAsLink, sal_Bool bSkipAttrForEmpty, ScTable* pTable);
    void        StartListeningInArea( SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2 );
    void        BroadcastInArea( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2 );

    void        CopyToTable(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            sal_uInt16 nFlags, sal_Bool bMarked, ScTable* pDestTab,
                            const ScMarkData* pMarkData = NULL,
                            sal_Bool bAsLink = sal_False, sal_Bool bColRowFlags = sal_True);
    void        UndoToTable(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            sal_uInt16 nFlags, sal_Bool bMarked, ScTable* pDestTab,
                            const ScMarkData* pMarkData = NULL);

    void        TransposeClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                ScTable* pTransClip, sal_uInt16 nFlags, sal_Bool bAsLink );

                //  Markierung von diesem Dokument
    void        MixMarked( const ScMarkData& rMark, sal_uInt16 nFunction,
                            sal_Bool bSkipEmpty, ScTable* pSrcTab );
    void        MixData( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            sal_uInt16 nFunction, sal_Bool bSkipEmpty, ScTable* pSrcTab );

    void        CopyData( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            SCCOL nDestCol, SCROW nDestRow, SCTAB nDestTab );

    void        CopyScenarioFrom( const ScTable* pSrcTab );
    void        CopyScenarioTo( ScTable* pDestTab ) const;
    sal_Bool        TestCopyScenarioTo( const ScTable* pDestTab ) const;
    void        MarkScenarioIn( ScMarkData& rMark, sal_uInt16 nNeededBits ) const;
    sal_Bool        HasScenarioRange( const ScRange& rRange ) const;
    void        InvalidateScenarioRanges();
    const ScRangeList* GetScenarioRanges() const;

    void        CopyUpdated( const ScTable* pPosTab, ScTable* pDestTab ) const;

    void        InvalidateTableArea();
    void        InvalidatePageBreaks();

    sal_Bool        GetCellArea( SCCOL& rEndCol, SCROW& rEndRow ) const;            // sal_False = leer
    sal_Bool        GetTableArea( SCCOL& rEndCol, SCROW& rEndRow ) const;
    sal_Bool        GetPrintArea( SCCOL& rEndCol, SCROW& rEndRow, sal_Bool bNotes ) const;
    sal_Bool        GetPrintAreaHor( SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rEndCol, sal_Bool bNotes ) const;
    sal_Bool        GetPrintAreaVer( SCCOL nStartCol, SCCOL nEndCol,
                                SCROW& rEndRow, sal_Bool bNotes ) const;

    sal_Bool        GetDataStart( SCCOL& rStartCol, SCROW& rStartRow ) const;

    void        ExtendPrintArea( OutputDevice* pDev,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL& rEndCol, SCROW nEndRow );

    void        GetDataArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow,
                             sal_Bool bIncludeOld, bool bOnlyDown ) const;

    bool        ShrinkToUsedDataArea( bool& o_bShrunk, SCCOL& rStartCol, SCROW& rStartRow,
                                      SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly ) const;

    SCSIZE      GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, ScDirection eDir );

    void        FindAreaPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY );
    void        GetNextPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY,
                                sal_Bool bMarked, sal_Bool bUnprotected, const ScMarkData& rMark );

    void        LimitChartArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow );

    sal_Bool        HasData( SCCOL nCol, SCROW nRow );
    sal_Bool        HasStringData( SCCOL nCol, SCROW nRow );
    sal_Bool        HasValueData( SCCOL nCol, SCROW nRow );
//UNUSED2008-05  sal_uInt16     GetErrorData(SCCOL nCol, SCROW nRow) const;
    sal_Bool        HasStringCells( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow ) const;

    sal_uInt16      GetErrCode( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetErrCode( rPos.Row() ) :
                            0;
                    }
//UNUSED2008-05  sal_uInt16     GetErrCode( SCCOL nCol, SCROW nRow ) const;

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
                                    ScDocument* pUndoDoc = NULL, sal_Bool bIncludeDraw = sal_True, bool bUpdateNoteCaptionPos = true );

    void        UpdateDrawRef( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz, bool bUpdateNoteCaptionPos = true );

    void        UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc );

    void        UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void        UpdateInsertTab(SCTAB nTable);
//UNUSED2008-05  void        UpdateInsertTabOnlyCells(SCTAB nTable);
    void        UpdateDeleteTab( SCTAB nTable, sal_Bool bIsMove, ScTable* pRefUndo = NULL );
    void        UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo, ScProgress& );
    void        UpdateCompile( sal_Bool bForceIfNameInUse = sal_False );
    void        SetTabNo(SCTAB nNewTab);
    sal_Bool        IsRangeNameInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                 sal_uInt16 nIndex) const;
    void        FindRangeNamesInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                 std::set<sal_uInt16>& rIndexes) const;
    void        ReplaceRangeNamesInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                      const ScRangeData::IndexMap& rMap );
    void        Fill( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                        double nStepValue, double nMaxValue);
    String      GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY );

    void        UpdateSelectionFunction( ScFunctionData& rData,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark );

    void        AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    sal_uInt16 nFormatNo );
    void        GetAutoFormatData(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, ScAutoFormatData& rData);
    void        ScReplaceTabsStr( String& rStr, const String& rSrch, const String& rRepl ); // aus sw
    sal_Bool        SearchAndReplace(const SvxSearchItem& rSearchItem,
                                SCCOL& rCol, SCROW& rRow, ScMarkData& rMark,
                                String& rUndoStr, ScDocument* pUndoDoc);

    void        FindMaxRotCol( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2 );

    void        GetBorderLines( SCCOL nCol, SCROW nRow,
                                const SvxBorderLine** ppLeft, const SvxBorderLine** ppTop,
                                const SvxBorderLine** ppRight, const SvxBorderLine** ppBottom ) const;

//UNUSED2009-05 sal_Bool        HasLines( const ScRange& rRange, Rectangle& rSizes ) const;
    bool        HasAttrib( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sal_uInt16 nMask ) const;
    sal_Bool        HasAttribSelection( const ScMarkData& rMark, sal_uInt16 nMask ) const;
    sal_Bool        ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow,
                                sal_Bool bRefresh, sal_Bool bAttrs );
    const SfxPoolItem*      GetAttr( SCCOL nCol, SCROW nRow, sal_uInt16 nWhich ) const;
    const ScPatternAttr*    GetPattern( SCCOL nCol, SCROW nRow ) const;
    const ScPatternAttr*    GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const;

    sal_uLong                   GetNumberFormat( const ScAddress& rPos ) const
                                {
                                    return ValidColRow(rPos.Col(),rPos.Row()) ?
                                        aCol[rPos.Col()].GetNumberFormat( rPos.Row() ) :
                                        0;
                                }
    sal_uLong                   GetNumberFormat( SCCOL nCol, SCROW nRow ) const;
    void                    MergeSelectionPattern( ScMergePatternState& rState,
                                                const ScMarkData& rMark, sal_Bool bDeep ) const;
    void                    MergePatternArea( ScMergePatternState& rState, SCCOL nCol1, SCROW nRow1,
                                                SCCOL nCol2, SCROW nRow2, sal_Bool bDeep ) const;
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
    void        ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, const ScPatternAttr& rAttr );
    void        SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr, sal_Bool bPutToPool = sal_False )
                    {
                        if (ValidColRow(rPos.Col(),rPos.Row()))
                            aCol[rPos.Col()].SetPattern( rPos.Row(), rAttr, bPutToPool );
                    }
    void        SetPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr, sal_Bool bPutToPool = sal_False );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, short nNewType );

    void        ApplyStyle( SCCOL nCol, SCROW nRow, const ScStyleSheet& rStyle );
    void        ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, const ScStyleSheet& rStyle );
    void        ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);
    void        ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const SvxBorderLine* pLine, sal_Bool bColorOnly );

    const ScStyleSheet* GetStyle( SCCOL nCol, SCROW nRow ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark, sal_Bool& rFound ) const;
    const ScStyleSheet* GetAreaStyle( sal_Bool& rFound, SCCOL nCol1, SCROW nRow1,
                                                    SCCOL nCol2, SCROW nRow2 ) const;

    void        StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, sal_Bool bRemoved,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY );

    sal_Bool        IsStyleSheetUsed( const ScStyleSheet& rStyle, sal_Bool bGatherAllStyles ) const;

    sal_Bool        ApplyFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, sal_Int16 nFlags );
    sal_Bool        RemoveFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, sal_Int16 nFlags );

    void        ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark );
    void        DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark );

    void        ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark );
    void        ChangeSelectionIndent( sal_Bool bIncrement, const ScMarkData& rMark );

    const ScRange*  GetRepeatColRange() const   { return pRepeatColRange; }
    const ScRange*  GetRepeatRowRange() const   { return pRepeatRowRange; }
    void            SetRepeatColRange( const ScRange* pNew );
    void            SetRepeatRowRange( const ScRange* pNew );

    sal_uInt16          GetPrintRangeCount() const          { return static_cast< sal_uInt16 >( aPrintRanges.size() ); }
    const ScRange*  GetPrintRange(sal_uInt16 nPos) const;
    /** Returns true, if the sheet is always printed. */
    sal_Bool            IsPrintEntireSheet() const          { return bPrintEntireSheet; }

    /** Removes all print ranges. */
    void            ClearPrintRanges();
    /** Adds a new print ranges. */
    void            AddPrintRange( const ScRange& rNew );
//UNUSED2009-05 /** Removes all old print ranges and sets the passed print ranges. */
//UNUSED2009-05 void            SetPrintRange( const ScRange& rNew );
    /** Marks the specified sheet to be printed completely. Deletes old print ranges! */
    void            SetPrintEntireSheet();

    void            FillPrintSaver( ScPrintSaverTab& rSaveTab ) const;
    void            RestorePrintRanges( const ScPrintSaverTab& rSaveTab );

    sal_uInt16      GetOptimalColWidth( SCCOL nCol, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    sal_Bool bFormula, const ScMarkData* pMarkData,
                                    sal_Bool bSimpleTextImport );
    sal_Bool        SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nExtra,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    sal_Bool bForce,
                                    ScProgress* pOuterProgress = NULL, sal_uLong nProgressStart = 0 );
    long        GetNeededSize( SCCOL nCol, SCROW nRow,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    sal_Bool bWidth, sal_Bool bTotalSize );
    void        SetColWidth( SCCOL nCol, sal_uInt16 nNewWidth );
    void        SetRowHeight( SCROW nRow, sal_uInt16 nNewHeight );
    sal_Bool        SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nNewHeight,
                                    double nPPTX, double nPPTY );

    /**
     * Set specified row height to specified ranges.  Don't check for drawing
     * objects etc.  Just set the row height.  Nothing else.
     *
     * Note that setting a new row height via this function will not
     * invalidate page breaks.
     */
    void        SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nNewHeight );

                        // nPPT fuer Test auf Veraenderung
    void        SetManualHeight( SCROW nStartRow, SCROW nEndRow, sal_Bool bManual );

    sal_uInt16      GetColWidth( SCCOL nCol ) const;
    SC_DLLPUBLIC sal_uInt16 GetRowHeight( SCROW nRow, SCROW* pStartRow = NULL, SCROW* pEndRow = NULL, bool bHiddenAsZero = true ) const;
    sal_uLong       GetRowHeight( SCROW nStartRow, SCROW nEndRow ) const;
    sal_uLong       GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow, double fScale ) const;
    sal_uLong       GetColOffset( SCCOL nCol ) const;
    sal_uLong       GetRowOffset( SCROW nRow ) const;

    /**
     * Get the last row such that the height of row 0 to the end row is as
     * high as possible without exceeding the specified height value.
     *
     * @param nHeight maximum desired height
     *
     * @return SCROW last row of the range within specified height.
     */
    SCROW       GetRowForHeight(sal_uLong nHeight) const;

    sal_uInt16      GetOriginalWidth( SCCOL nCol ) const;
    sal_uInt16      GetOriginalHeight( SCROW nRow ) const;

    sal_uInt16      GetCommonWidth( SCCOL nEndCol );

    SCROW       GetHiddenRowCount( SCROW nRow );

    void        ShowCol(SCCOL nCol, bool bShow);
    void        ShowRow(SCROW nRow, bool bShow);
    void        DBShowRow(SCROW nRow, bool bShow);

    void        ShowRows(SCROW nRow1, SCROW nRow2, bool bShow);
    void        DBShowRows(SCROW nRow1, SCROW nRow2, bool bShow, bool bSetFlags);   // if bSetFlags=false, no SetRowHidden/SetRowFiltered

    void        SetColFlags( SCCOL nCol, sal_uInt8 nNewFlags );
    void        SetRowFlags( SCROW nRow, sal_uInt8 nNewFlags );
    void        SetRowFlags( SCROW nStartRow, SCROW nEndRow, sal_uInt8 nNewFlags );

                /// @return  the index of the last row with any set flags (auto-pagebreak is ignored).
    SCROW      GetLastFlaggedRow() const;

                /// @return  the index of the last changed column (flags and column width, auto pagebreak is ignored).
    SCCOL      GetLastChangedCol() const;
                /// @return  the index of the last changed row (flags and row height, auto pagebreak is ignored).
    SCROW      GetLastChangedRow() const;

    sal_uInt8       GetColFlags( SCCOL nCol ) const;
    sal_uInt8       GetRowFlags( SCROW nRow ) const;

    const ScBitMaskCompressedArray< SCROW, sal_uInt8> * GetRowFlagsArray() const
                    { return pRowFlags; }

    sal_Bool        UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, sal_Bool bShow );
    sal_Bool        UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, sal_Bool bShow );

    void        UpdatePageBreaks( const ScRange* pUserArea );
    void        RemoveManualBreaks();
    sal_Bool        HasManualBreaks() const;
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
    bool        RowHidden(SCROW nRow, SCROW& rLastRow) const;
    bool        HasHiddenRows(SCROW nStartRow, SCROW nEndRow) const;
    bool        ColHidden(SCCOL nCol, SCCOL& rLastCol) const;
    bool        ColHidden(SCCOL nCol, SCCOL* pFirstCol = NULL, SCCOL* pLastCol = NULL) const;
    void        SetRowHidden(SCROW nStartRow, SCROW nEndRow, bool bHidden);
    void        SetColHidden(SCCOL nStartCol, SCCOL nEndCol, bool bHidden);
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

    void        Sort(const ScSortParam& rSortParam, sal_Bool bKeepQuery);
    sal_Bool        ValidQuery(SCROW nRow, const ScQueryParam& rQueryParam,
                    sal_Bool* pSpecial = NULL, ScBaseCell* pCell = NULL,
                    sal_Bool* pbTestEqualCondition = NULL );
    void        TopTenQuery( ScQueryParam& );
    SCSIZE      Query(ScQueryParam& rQueryParam, sal_Bool bKeepSub);
    sal_Bool        CreateQueryParam(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);

    void        GetFilterEntries(SCCOL nCol, SCROW nRow1, SCROW nRow2, TypedScStrCollection& rStrings, bool& rHasDates);
    void        GetFilteredFilterEntries( SCCOL nCol, SCROW nRow1, SCROW nRow2, const ScQueryParam& rParam, TypedScStrCollection& rStrings, bool& rHasDates );
    sal_Bool        GetDataEntries(SCCOL nCol, SCROW nRow, TypedScStrCollection& rStrings, sal_Bool bLimit);

    sal_Bool        HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );
    sal_Bool        HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );

    void        DoColResize( SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd );


    sal_Int32   GetMaxStringLen( SCCOL nCol,
                                    SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const;
    xub_StrLen  GetMaxNumberStringLen( sal_uInt16& nPrecision,
                                       SCCOL nCol,
                                       SCROW nRowStart, SCROW nRowEnd ) const;

    void        FindConditionalFormat( sal_uLong nKey, ScRangeList& rRanges );

    void        IncRecalcLevel() { ++nRecalcLvl; }
    void        DecRecalcLevel( bool bUpdateNoteCaptionPos = true ) { if (!--nRecalcLvl) SetDrawPageSize(true, bUpdateNoteCaptionPos); }

    sal_Bool        IsSortCollatorGlobal() const;
    void        InitSortCollator( const ScSortParam& rPar );
    void        DestroySortCollator();

private:
    void        FillSeries( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd,
                                FillDateCmd eFillDateCmd,
                                double nStepValue, double nMaxValue, sal_uInt16 nMinDigits,
                                sal_Bool bAttribs, ScProgress& rProgress );
    void        FillAnalyse( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                FillCmd& rCmd, FillDateCmd& rDateCmd,
                                double& rInc, sal_uInt16& rMinDigits,
                                ScUserListData*& rListData, sal_uInt16& rListIndex);
    void        FillAuto( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        sal_uLong nFillCount, FillDir eFillDir, ScProgress& rProgress );

    sal_Bool        ValidNextPos( SCCOL nCol, SCROW nRow, const ScMarkData& rMark,
                                sal_Bool bMarked, sal_Bool bUnprotected );

    void        AutoFormatArea(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                const ScPatternAttr& rAttr, sal_uInt16 nFormatNo);
    void        GetAutoFormatAttr(SCCOL nCol, SCROW nRow, sal_uInt16 nIndex, ScAutoFormatData& rData);
    void        GetAutoFormatFrame(SCCOL nCol, SCROW nRow, sal_uInt16 nFlags, sal_uInt16 nIndex, ScAutoFormatData& rData);
    sal_Bool        SearchCell(const SvxSearchItem& rSearchItem, SCCOL nCol, SCROW nRow,
                            const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc);
    sal_Bool        Search(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc);
    sal_Bool        SearchAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                        String& rUndoStr, ScDocument* pUndoDoc);
    sal_Bool        Replace(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc);
    sal_Bool        ReplaceAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                            String& rUndoStr, ScDocument* pUndoDoc);

    sal_Bool        SearchStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                                ScMarkData& rMark);
    sal_Bool        ReplaceStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                                ScMarkData& rMark, sal_Bool bIsUndo);
    sal_Bool        SearchAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark);
    sal_Bool        ReplaceAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                                ScDocument* pUndoDoc);

                                // benutzen globalen SortParam:
    sal_Bool        IsSorted(SCCOLROW nStart, SCCOLROW nEnd);
    void        DecoladeRow( ScSortInfoArray*, SCROW nRow1, SCROW nRow2 );
    void        SwapCol(SCCOL nCol1, SCCOL nCol2);
    void        SwapRow(SCROW nRow1, SCROW nRow2);
    short       CompareCell( sal_uInt16 nSort,
                    ScBaseCell* pCell1, SCCOL nCell1Col, SCROW nCell1Row,
                    ScBaseCell* pCell2, SCCOL nCell2Col, SCROW nCell2Row );
    short       Compare(SCCOLROW nIndex1, SCCOLROW nIndex2);
    short       Compare( ScSortInfoArray*, SCCOLROW nIndex1, SCCOLROW nIndex2);
    ScSortInfoArray*    CreateSortInfoArray( SCCOLROW nInd1, SCCOLROW nInd2 );
    void        QuickSort( ScSortInfoArray*, SCsCOLROW nLo, SCsCOLROW nHi);
    void        SortReorder( ScSortInfoArray*, ScProgress& );

    sal_Bool        CreateExcelQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);
    sal_Bool        CreateStarQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);
    void        GetUpperCellString(SCCOL nCol, SCROW nRow, String& rStr);

    sal_Bool        RefVisible(ScFormulaCell* pCell);

    sal_Bool        IsEmptyLine(SCROW nRow, SCCOL nStartCol, SCCOL nEndCol);

    void        IncDate(double& rVal, sal_uInt16& nDayOfMonth, double nStep, FillDateCmd eCmd);
    void        FillFormula(sal_uLong& nFormulaCounter, sal_Bool bFirst, ScFormulaCell* pSrcCell,
                            SCCOL nDestCol, SCROW nDestRow, sal_Bool bLast );
    void        UpdateInsertTabAbs(SCTAB nNewPos);
    sal_Bool        GetNextSpellingCell(SCCOL& rCol, SCROW& rRow, sal_Bool bInSel,
                                    const ScMarkData& rMark) const;
    sal_Bool        GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark );
    void        SetDrawPageSize( bool bResetStreamValid = true, bool bUpdateNoteCaptionPos = true );
    sal_Bool        TestTabRefAbs(SCTAB nTable);
    void        CompileDBFormula();
    void        CompileDBFormula( sal_Bool bCreateFormulaString );
    void        CompileNameFormula( sal_Bool bCreateFormulaString );
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
                                     sal_Bool bNumFormatChanged, sal_Bool bBroadcast );

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


