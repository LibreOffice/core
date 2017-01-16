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

#ifndef INCLUDED_SC_INC_TABLE_HXX
#define INCLUDED_SC_INC_TABLE_HXX

#include <vector>
#include <utility>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "attarray.hxx"
#include "column.hxx"
#include "colcontainer.hxx"
#include "sortparam.hxx"
#include "compressedarray.hxx"
#include "postit.hxx"
#include "types.hxx"
#include "cellvalue.hxx"
#include <formula/types.hxx>
#include "calcmacros.hxx"
#include "formula/errorcodes.hxx"

#include <set>
#include <map>
#include <memory>

namespace utl {
    class TextSearch;
}

namespace com { namespace sun { namespace star {
    namespace sheet {
        struct TablePageBreakData;
    }
} } }

namespace formula { struct VectorRefArray; }
namespace sc {

struct FormulaGroupContext;
class StartListeningContext;
class EndListeningContext;
class CopyFromClipContext;
class CopyToClipContext;
class CopyToDocContext;
class MixDocContext;
class ColumnSpanSet;
class ColumnSet;
struct ColumnBlockPosition;
struct RefUpdateContext;
struct RefUpdateInsertTabContext;
struct RefUpdateDeleteTabContext;
struct RefUpdateMoveTabContext;
struct NoteEntry;
class DocumentStreamAccess;
class CellValues;
class TableValues;
class RowHeightContext;
class CompileFormulaContext;
struct SetFormulaDirtyContext;
class RefMovedHint;
struct ReorderParam;

}

class SfxItemSet;
class SfxStyleSheetBase;
class SvxBoxInfoItem;
class SvxBoxItem;
class SvxSearchItem;

class ScAutoFormatData;
class ScDocument;
class ScEditDataArray;
class ScFormulaCell;
class ScOutlineTable;
class ScPrintSaverTab;
class ScProgress;
class ScRangeList;
class ScSheetEvents;
class ScSortInfoArray;
class ScConditionalFormat;
class ScConditionalFormatList;
class ScStyleSheet;
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
class ScDBData;
class ScDocumentImport;
class ScHint;

class ScTable
{
private:
    typedef ::std::vector< ScRange > ScRangeVec;

    ScColContainer  aCol;

    OUString aName;
    OUString aCodeName;
    OUString aComment;

    OUString        aLinkDoc;
    OUString        aLinkFlt;
    OUString        aLinkOpt;
    OUString        aLinkTab;
    sal_uLong       nLinkRefreshDelay;
    ScLinkMode      nLinkMode;

    // page style template
    OUString        aPageStyle;
    Size            aPageSizeTwips;                 // size of the print-page
    SCCOL           nRepeatStartX;                  // repeating rows/columns
    SCCOL           nRepeatEndX;                    // REPEAT_NONE, if not used
    SCROW           nRepeatStartY;
    SCROW           nRepeatEndY;

    std::unique_ptr<ScTableProtection> pTabProtection;

    sal_uInt16*         pColWidth;
    std::unique_ptr<ScFlatUInt16RowSegments> mpRowHeights;

    CRFlags*            pColFlags;
    ScBitMaskCompressedArray< SCROW, CRFlags>*     pRowFlags;
    std::unique_ptr<ScFlatBoolColSegments>  mpHiddenCols;
    std::unique_ptr<ScFlatBoolRowSegments>  mpHiddenRows;
    std::unique_ptr<ScFlatBoolColSegments>  mpFilteredCols;
    std::unique_ptr<ScFlatBoolRowSegments>  mpFilteredRows;

    ::std::set<SCROW>                      maRowPageBreaks;
    ::std::set<SCROW>                      maRowManualBreaks;
    ::std::set<SCCOL>                      maColPageBreaks;
    ::std::set<SCCOL>                      maColManualBreaks;

    ScOutlineTable* pOutlineTable;

    ScSheetEvents*  pSheetEvents;

    mutable SCCOL nTableAreaX;
    mutable SCROW nTableAreaY;

    SCTAB           nTab;
    ScDocument*     pDocument;
    utl::TextSearch*    pSearchText;

    mutable OUString aUpperName;             // #i62977# filled only on demand, reset in SetName

    // sort parameter to minimize stack size of quicksort
    ScSortParam     aSortParam;
    CollatorWrapper*    pSortCollator;

    ScRangeVec      aPrintRanges;

    ScRange*        pRepeatColRange;
    ScRange*        pRepeatRowRange;

    sal_uInt16          nLockCount;

    ScRangeList*    pScenarioRanges;
    Color           aScenarioColor;
    Color           aTabBgColor;
    ScScenarioFlags nScenarioFlags;
    ScDBData*       pDBDataNoName;
    mutable ScRangeName* mpRangeName;

    std::unique_ptr<ScConditionalFormatList> mpCondFormatList;

    bool            bScenario:1;
    bool            bLayoutRTL:1;
    bool            bLoadingRTL:1;
    bool            bPageSizeValid:1;
    mutable bool    bTableAreaValid:1;
    bool            bVisible:1;
    bool            bStreamValid:1;
    bool            bPendingRowHeights:1;
    bool            bCalcNotification:1;
    bool            bGlobalKeepQuery:1;
    bool            bPrintEntireSheet:1;
    bool            bActiveScenario:1;
    bool            mbPageBreaksValid:1;
    bool            mbForceBreaks:1;

    ScAttrArray     aNextColAttrArray;

friend class ScDocument;                    // for FillInfo
friend class ScColumn;
friend class ScValueIterator;
friend class ScHorizontalValueIterator;
friend class ScDBQueryDataIterator;
friend class ScFormulaGroupIterator;
friend class ScCellIterator;
friend class ScQueryCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScDocAttrIterator;
friend class ScAttrRectIterator;
friend class ScColumnTextWidthIterator;
friend class ScDocumentImport;
friend class sc::DocumentStreamAccess;
friend class sc::ColumnSpanSet;
friend class sc::EditTextIterator;
friend class sc::FormulaGroupAreaListener;

public:
                ScTable( ScDocument* pDoc, SCTAB nNewTab, const OUString& rNewName,
                         bool bColInfo = true, bool bRowInfo = true );
                ~ScTable();
                ScTable(const ScTable&) = delete;
    ScTable&    operator=(const ScTable&) = delete;

    ScDocument& GetDoc() { return *pDocument;}
    const ScDocument& GetDoc() const { return *pDocument;}
    SCTAB GetTab() const { return nTab; }

    ScOutlineTable* GetOutlineTable()               { return pOutlineTable; }

    sal_uLong       GetCellCount() const;
    sal_uLong       GetWeightedCount() const;
    sal_uLong       GetCodeCount() const;       // RPN code in formula

    sal_uInt16 GetTextWidth(SCCOL nCol, SCROW nRow) const;

    bool        SetOutlineTable( const ScOutlineTable* pNewOutline );
    void        StartOutlineTable();

    void        DoAutoOutline( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );

    bool        TestRemoveSubTotals( const ScSubTotalParam& rParam );
    void        RemoveSubTotals( ScSubTotalParam& rParam );
    bool        DoSubTotals( ScSubTotalParam& rParam );

    const ScSheetEvents* GetSheetEvents() const              { return pSheetEvents; }
    void        SetSheetEvents( const ScSheetEvents* pNew );

    bool        IsVisible() const                            { return bVisible; }
    void        SetVisible( bool bVis );

    bool        IsStreamValid() const                        { return bStreamValid; }
    void        SetStreamValid( bool bSet, bool bIgnoreLock = false );

    SAL_WARN_UNUSED_RESULT inline bool IsColValid( SCCOL nScCol ) const
    {
        return nScCol >= static_cast< SCCOL >( 0 ) && nScCol < aCol.size();
    }
    SAL_WARN_UNUSED_RESULT inline bool IsColRowValid( SCCOL nScCol, SCROW nScRow ) const
    {
        return IsColValid( nScCol ) && ValidRow( nScRow );
    }
    SAL_WARN_UNUSED_RESULT inline bool IsColRowTabValid( SCCOL nScCol, SCROW nScRow, SCTAB nScTab ) const
    {
        return IsColValid( nScCol ) && ValidRow( nScRow ) && ValidTab( nScTab );
    }

    bool        IsPendingRowHeights() const                  { return bPendingRowHeights; }
    void        SetPendingRowHeights( bool bSet );

    bool        GetCalcNotification() const                  { return bCalcNotification; }
    void        SetCalcNotification( bool bSet );

    bool        IsLayoutRTL() const                          { return bLayoutRTL; }
    bool        IsLoadingRTL() const                         { return bLoadingRTL; }
    void        SetLayoutRTL( bool bSet );
    void        SetLoadingRTL( bool bSet );

    bool        IsScenario() const                           { return bScenario; }
    void        SetScenario( bool bFlag );
    void        GetScenarioComment( OUString& rComment) const  { rComment = aComment; }
    void        SetScenarioComment( const OUString& rComment ) { aComment = rComment; }
    const Color& GetScenarioColor() const                    { return aScenarioColor; }
    void         SetScenarioColor(const Color& rNew)         { aScenarioColor = rNew; }
    const Color& GetTabBgColor() const                       { return aTabBgColor; }
    void         SetTabBgColor(const Color& rColor);
    ScScenarioFlags GetScenarioFlags() const                 { return nScenarioFlags; }
    void        SetScenarioFlags(ScScenarioFlags nNew)       { nScenarioFlags = nNew; }
    void        SetActiveScenario(bool bSet)                 { bActiveScenario = bSet; }
    bool        IsActiveScenario() const                     { return bActiveScenario; }

    ScLinkMode  GetLinkMode() const                          { return nLinkMode; }
    bool        IsLinked() const                             { return nLinkMode != ScLinkMode::NONE; }
    const OUString& GetLinkDoc() const                       { return aLinkDoc; }
    const OUString& GetLinkFlt() const                       { return aLinkFlt; }
    const OUString& GetLinkOpt() const                       { return aLinkOpt; }
    const OUString& GetLinkTab() const                       { return aLinkTab; }
    sal_uLong   GetLinkRefreshDelay() const                  { return nLinkRefreshDelay; }

    void        SetLink( ScLinkMode nMode, const OUString& rDoc, const OUString& rFlt,
                        const OUString& rOpt, const OUString& rTab, sal_uLong nRefreshDelay );

    sal_Int64   GetHashCode () const;

    void        GetName( OUString& rName ) const;
    void        SetName( const OUString& rNewName );

    void        SetAnonymousDBData(ScDBData* pDBData);
    ScDBData*   GetAnonymousDBData() { return pDBDataNoName;}

    void        GetCodeName( OUString& rName ) const {  rName = aCodeName; }
    void        SetCodeName( const OUString& rNewName ) { aCodeName = rNewName; }

    const OUString& GetUpperName() const;

    const OUString&   GetPageStyle() const                    { return aPageStyle; }
    void            SetPageStyle( const OUString& rName );
    void            PageStyleModified( const OUString& rNewName );

    bool            IsProtected() const;
    void            SetProtection(const ScTableProtection* pProtect);
    ScTableProtection* GetProtection();

    Size            GetPageSize() const;
    void            SetPageSize( const Size& rSize );
    void            SetRepeatArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow );

    void        LockTable();
    void        UnlockTable();

    bool        IsBlockEditable( SCCOL nCol1, SCROW nRow1, SCCOL nCol2,
                        SCROW nRow2, bool* pOnlyNotBecauseOfMatrix = nullptr ) const;
    bool        IsSelectionEditable( const ScMarkData& rMark,
                        bool* pOnlyNotBecauseOfMatrix = nullptr ) const;

    bool        HasBlockMatrixFragment( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;
    bool        HasSelectionMatrixFragment( const ScMarkData& rMark ) const;

    bool        IsBlockEmpty( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bIgnoreNotes ) const;

    bool        SetString( SCCOL nCol, SCROW nRow, SCTAB nTab, const OUString& rString,
                           ScSetStringParam* pParam = nullptr );

    bool SetEditText( SCCOL nCol, SCROW nRow, EditTextObject* pEditText );
    void SetEditText( SCCOL nCol, SCROW nRow, const EditTextObject& rEditText, const SfxItemPool* pEditPool );
    SCROW GetFirstEditTextRow( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;

    void SetEmptyCell( SCCOL nCol, SCROW nRow );
    void SetFormula(
        SCCOL nCol, SCROW nRow, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram );
    void SetFormula(
        SCCOL nCol, SCROW nRow, const OUString& rFormula, formula::FormulaGrammar::Grammar eGram );

    /**
     * Takes ownership of pCell
     *
     * @return pCell if it was successfully inserted, NULL otherwise. pCell
     *         is deleted automatically on failure to insert.
     */
    ScFormulaCell* SetFormulaCell( SCCOL nCol, SCROW nRow, ScFormulaCell* pCell );

    bool SetFormulaCells( SCCOL nCol, SCROW nRow, std::vector<ScFormulaCell*>& rCells );

    bool HasFormulaCell( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;

    svl::SharedString GetSharedString( SCCOL nCol, SCROW nRow ) const;

    void        SetValue( SCCOL nCol, SCROW nRow, const double& rVal );
    void        SetValues( SCCOL nCol, SCROW nRow, const std::vector<double>& rVals );
    void        SetError( SCCOL nCol, SCROW nRow, FormulaError nError);
    SCSIZE      GetPatternCount( SCCOL nCol ) const;
    SCSIZE      GetPatternCount( SCCOL nCol, SCROW nRow1, SCROW nRow2 ) const;
    bool        ReservePatternCount( SCCOL nCol, SCSIZE nReserve );

    void        SetRawString( SCCOL nCol, SCROW nRow, const svl::SharedString& rStr );
    void        GetString( SCCOL nCol, SCROW nRow, OUString& rString ) const;
    double*     GetValueCell( SCCOL nCol, SCROW nRow );
    void        GetInputString( SCCOL nCol, SCROW nRow, OUString& rString ) const;
    double      GetValue( SCCOL nCol, SCROW nRow ) const;
    const EditTextObject* GetEditText( SCCOL nCol, SCROW nRow ) const;
    void RemoveEditTextCharAttribs( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr );
    void GetFormula( SCCOL nCol, SCROW nRow, OUString& rFormula ) const;
    const ScFormulaCell* GetFormulaCell( SCCOL nCol, SCROW nRow ) const;
    ScFormulaCell* GetFormulaCell( SCCOL nCol, SCROW nRow );

    CellType    GetCellType( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetCellType( rPos.Row() ) :
                            CELLTYPE_NONE;
                    }
    CellType    GetCellType( SCCOL nCol, SCROW nRow ) const;
    ScRefCellValue GetCellValue( SCCOL nCol, SCROW nRow ) const;

    void        GetFirstDataPos(SCCOL& rCol, SCROW& rRow) const;
    void        GetLastDataPos(SCCOL& rCol, SCROW& rRow) const;

    ScPostIt* ReleaseNote( SCCOL nCol, SCROW nRow );

    size_t GetNoteCount( SCCOL nCol ) const;
    SCROW GetNotePosition( SCCOL nCol, size_t nIndex ) const;
    void CreateAllNoteCaptions();
    void ForgetNoteCaptions( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bPreserveData );

    void GetAllNoteEntries( std::vector<sc::NoteEntry>& rNotes ) const;
    void GetNotesInRange( const ScRange& rRange, std::vector<sc::NoteEntry>& rNotes ) const;
    bool ContainsNotesInRange( const ScRange& rRange ) const;

    bool TestInsertRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize ) const;
    void        InsertRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize );
    void DeleteRow(
        const sc::ColumnSet& rRegroupCols, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize,
        bool* pUndoOutline, std::vector<ScAddress>* pGroupPos );

    bool        TestInsertCol( SCROW nStartRow, SCROW nEndRow, SCSIZE nSize ) const;
    void InsertCol(
        const sc::ColumnSet& rRegroupCols, SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize );
    void DeleteCol(
        const sc::ColumnSet& rRegroupCols, SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize, bool* pUndoOutline );

    void DeleteArea(
        SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, InsertDeleteFlags nDelFlag,
        bool bBroadcast = true, sc::ColumnSpanSet* pBroadcastSpans = nullptr );

    void CopyToClip( sc::CopyToClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScTable* pTable );
    void CopyToClip( sc::CopyToClipContext& rCxt, const ScRangeList& rRanges, ScTable* pTable );

    void CopyStaticToDocument(
        SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, const SvNumberFormatterMergeMap& rMap,
        ScTable* pDestTab );

    void CopyCellToDocument( SCCOL nSrcCol, SCROW nSrcRow, SCCOL nDestCol, SCROW nDestRow, ScTable& rDestTab );

    bool InitColumnBlockPosition( sc::ColumnBlockPosition& rBlockPos, SCCOL nCol );

    void DeleteBeforeCopyFromClip(
        sc::CopyFromClipContext& rCxt, const ScTable& rClipTab, sc::ColumnSpanSet& rBroadcastSpans );

    void CopyOneCellFromClip(
        sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, SCROW nSrcRow, ScTable* pSrcTab );

    void CopyFromClip(
        sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        SCsCOL nDx, SCsROW nDy, ScTable* pTable );

    void StartListeningFormulaCells(
        sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt,
        SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    void SetDirtyFromClip(
        SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sc::ColumnSpanSet& rBroadcastSpans );

    void CopyToTable(
        sc::CopyToDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        InsertDeleteFlags nFlags, bool bMarked, ScTable* pDestTab,
        const ScMarkData* pMarkData = nullptr, bool bAsLink = false, bool bColRowFlags = true,
        bool bGlobalNamesToLocal = false, bool bCopyCaptions = true );

    void CopyCaptionsToTable( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScTable* pDestTab, bool bCloneCaption );

    void UndoToTable(
        sc::CopyToDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        InsertDeleteFlags nFlags, bool bMarked, ScTable* pDestTab );

    void        CopyConditionalFormat( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            SCsCOL nDx, SCsROW nDy, ScTable* pTable);
    void        TransposeClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                ScTable* pTransClip, InsertDeleteFlags nFlags, bool bAsLink );

                // mark of this document
    void MixMarked(
        sc::MixDocContext& rCxt, const ScMarkData& rMark, ScPasteFunc nFunction,
        bool bSkipEmpty, const ScTable* pSrcTab );

    void MixData(
        sc::MixDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        ScPasteFunc nFunction, bool bSkipEmpty, const ScTable* pSrcTab );

    void        CopyData( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            SCCOL nDestCol, SCROW nDestRow, SCTAB nDestTab );

    void        CopyScenarioFrom( const ScTable* pSrcTab );
    void        CopyScenarioTo( ScTable* pDestTab ) const;
    bool        TestCopyScenarioTo( const ScTable* pDestTab ) const;
    void        MarkScenarioIn(ScMarkData& rMark, ScScenarioFlags nNeededBits) const;
    bool        HasScenarioRange( const ScRange& rRange ) const;
    void        InvalidateScenarioRanges();
    const ScRangeList* GetScenarioRanges() const;

    void        CopyUpdated( const ScTable* pPosTab, ScTable* pDestTab ) const;

    void        InvalidateTableArea();
    void        InvalidatePageBreaks();

    bool        GetCellArea( SCCOL& rEndCol, SCROW& rEndRow ) const;            // FALSE = empty
    bool        GetTableArea( SCCOL& rEndCol, SCROW& rEndRow ) const;
    bool        GetPrintArea( SCCOL& rEndCol, SCROW& rEndRow, bool bNotes ) const;
    bool        GetPrintAreaHor( SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rEndCol, bool bNotes ) const;
    bool        GetPrintAreaVer( SCCOL nStartCol, SCCOL nEndCol,
                                SCROW& rEndRow, bool bNotes ) const;

    bool        GetDataStart( SCCOL& rStartCol, SCROW& rStartRow ) const;

    void        ExtendPrintArea( OutputDevice* pDev,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL& rEndCol, SCROW nEndRow );

    void        GetDataArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow,
                             bool bIncludeOld, bool bOnlyDown ) const;

    bool        ShrinkToUsedDataArea( bool& o_bShrunk, SCCOL& rStartCol, SCROW& rStartRow,
                                      SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly,
                                      bool bStickyTopRow, bool bStickyLeftCol, bool bConsiderCellNotes=false ) const;

    SCROW GetLastDataRow( SCCOL nCol1, SCCOL nCol2, SCROW nLastRow ) const;

    SCSIZE      GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, ScDirection eDir ) const;

    void        FindAreaPos( SCCOL& rCol, SCROW& rRow, ScMoveDirection eDirection ) const;
    void        GetNextPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY,
                                bool bMarked, bool bUnprotected, const ScMarkData& rMark ) const;

    void        LimitChartArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow ) const;

    bool        HasData( SCCOL nCol, SCROW nRow ) const;
    bool        HasStringData( SCCOL nCol, SCROW nRow ) const;
    bool        HasValueData( SCCOL nCol, SCROW nRow ) const;
    bool        HasStringCells( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow ) const;

    FormulaError    GetErrCode( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetErrCode( rPos.Row() ) :
                            FormulaError::NONE;
                    }

    void        ResetChanged( const ScRange& rRange );

    void CheckVectorizationState();
    void SetAllFormulasDirty( const sc::SetFormulaDirtyContext& rCxt );
    void        SetDirty( const ScRange&, ScColumn::BroadcastMode );
    void        SetDirtyAfterLoad();
    void        SetDirtyVar();
    void        SetTableOpDirty( const ScRange& );
    void        CalcAll();
    void CalcAfterLoad( sc::CompileFormulaContext& rCxt, bool bStartListening );
    void CompileAll( sc::CompileFormulaContext& rCxt );
    void CompileXML( sc::CompileFormulaContext& rCxt, ScProgress& rProgress );

    /** Broadcast single broadcasters in range, without explicitly setting
        anything dirty, not doing area broadcasts.
        @param rHint address is modified to adapt to the actual broadcasted
                position on each iteration and upon return points to the last
                position broadcasted. */
    bool BroadcastBroadcasters( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScHint& rHint );

    bool CompileErrorCells( sc::CompileFormulaContext& rCxt, FormulaError nErrCode );

    void UpdateReference(
        sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc = nullptr,
        bool bIncludeDraw = true, bool bUpdateNoteCaptionPos = true );

    void        UpdateDrawRef( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz, bool bUpdateNoteCaptionPos = true );

    void        UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc );

    void        UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt );
    void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt );
    void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt, SCTAB nTabNo, ScProgress* pProgress );
    void        UpdateCompile( bool bForceIfNameInUse = false );
    void        SetTabNo(SCTAB nNewTab);
    void        FindRangeNamesInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                 sc::UpdatedRangeNames& rIndexes) const;
    void        Fill( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                        double nStepValue, double nMaxValue, ScProgress* pProgress);
    OUString    GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY );

    void UpdateSelectionFunction( ScFunctionData& rData, const ScMarkData& rMark );

    void        AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    sal_uInt16 nFormatNo );
    void        GetAutoFormatData(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, ScAutoFormatData& rData);
    bool        SearchAndReplace(
        const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark,
        ScRangeList& rMatchedRanges, OUString& rUndoStr, ScDocument* pUndoDoc);

    void        FindMaxRotCol( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2 );

    bool        HasAttrib( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, HasAttrFlags nMask ) const;
    bool        HasAttribSelection( const ScMarkData& rMark, HasAttrFlags nMask ) const;
    bool IsMerged( SCCOL nCol, SCROW nRow ) const;
    bool        ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow,
                                bool bRefresh );
    const SfxPoolItem*      GetAttr( SCCOL nCol, SCROW nRow, sal_uInt16 nWhich ) const;
    const ScPatternAttr*    GetPattern( SCCOL nCol, SCROW nRow ) const;
    const ScPatternAttr*    GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const;

    sal_uInt32 GetNumberFormat( const ScAddress& rPos ) const;
    sal_uInt32 GetNumberFormat( SCCOL nCol, SCROW nRow ) const;
    sal_uInt32 GetNumberFormat( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const;

    void SetNumberFormat( SCCOL nCol, SCROW nRow, sal_uInt32 nNumberFormat );

    void                    MergeSelectionPattern( ScMergePatternState& rState,
                                                const ScMarkData& rMark, bool bDeep ) const;
    void                    MergePatternArea( ScMergePatternState& rState, SCCOL nCol1, SCROW nRow1,
                                                SCCOL nCol2, SCROW nRow2, bool bDeep ) const;
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
                                  const ScPatternAttr& rAttr, ScEditDataArray* pDataArray = nullptr );

    void        SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr )
                    {
                        if (ValidColRow(rPos.Col(),rPos.Row()))
                            aCol[rPos.Col()].SetPattern( rPos.Row(), rAttr );
                    }
    void        SetPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, short nNewType );
    void        AddCondFormatData( const ScRangeList& rRange, sal_uInt32 nIndex );
    void        RemoveCondFormatData( const ScRangeList& rRange, sal_uInt32 nIndex );

    void        ApplyStyle( SCCOL nCol, SCROW nRow, const ScStyleSheet* rStyle );
    void        ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, const ScStyleSheet& rStyle );
    void        ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);
    void        ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const ::editeng::SvxBorderLine* pLine, bool bColorOnly );

    const ScStyleSheet* GetStyle( SCCOL nCol, SCROW nRow ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark, bool& rFound ) const;
    const ScStyleSheet* GetAreaStyle( bool& rFound, SCCOL nCol1, SCROW nRow1,
                                                    SCCOL nCol2, SCROW nRow2 ) const;

    void        StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, bool bRemoved,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY );

    bool        IsStyleSheetUsed( const ScStyleSheet& rStyle ) const;

    bool        ApplyFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, ScMF nFlags );
    bool        RemoveFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, ScMF nFlags );

    void        ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark, ScEditDataArray* pDataArray = nullptr );
    void        DeleteSelection( InsertDeleteFlags nDelFlag, const ScMarkData& rMark, bool bBroadcast = true );

    void        ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark );
    void        ChangeSelectionIndent( bool bIncrement, const ScMarkData& rMark );

    const ScRange*  GetRepeatColRange() const   { return pRepeatColRange; }
    const ScRange*  GetRepeatRowRange() const   { return pRepeatRowRange; }
    void            SetRepeatColRange( const ScRange* pNew );
    void            SetRepeatRowRange( const ScRange* pNew );

    sal_uInt16          GetPrintRangeCount() const          { return static_cast< sal_uInt16 >( aPrintRanges.size() ); }
    const ScRange*  GetPrintRange(sal_uInt16 nPos) const;
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

    sal_uInt16      GetOptimalColWidth( SCCOL nCol, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    bool bFormula, const ScMarkData* pMarkData,
                                    const ScColWidthParam* pParam );
    bool SetOptimalHeight(
        sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow,
        ScProgress* pOuterProgress = nullptr, sal_uLong nProgressStart = 0 );

    void SetOptimalHeightOnly(
        sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow,
        ScProgress* pOuterProgress = nullptr, sal_uLong nProgressStart = 0 );

    long        GetNeededSize( SCCOL nCol, SCROW nRow,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    bool bWidth, bool bTotalSize );
    void        SetColWidth( SCCOL nCol, sal_uInt16 nNewWidth );
    void        SetColWidthOnly( SCCOL nCol, sal_uInt16 nNewWidth );
    void        SetRowHeight( SCROW nRow, sal_uInt16 nNewHeight );
    bool        SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nNewHeight,
                                    double nPPTX, double nPPTY );

    /**
     * Set specified row height to specified ranges.  Don't check for drawing
     * objects etc.  Just set the row height.  Nothing else.
     *
     * Note that setting a new row height via this function will not
     * invalidate page breaks.
     */
    void        SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nNewHeight );

                        // nPPT to test for modification
    void        SetManualHeight( SCROW nStartRow, SCROW nEndRow, bool bManual );

    sal_uInt16      GetColWidth( SCCOL nCol, bool bHiddenAsZero = true ) const;
    sal_uLong GetColWidth( SCCOL nStartCol, SCCOL nEndCol ) const;
    SC_DLLPUBLIC sal_uInt16 GetRowHeight( SCROW nRow, SCROW* pStartRow, SCROW* pEndRow = nullptr, bool bHiddenAsZero = true ) const;
    sal_uLong       GetRowHeight( SCROW nStartRow, SCROW nEndRow, bool bHiddenAsZero = true ) const;
    sal_uLong       GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow, double fScale ) const;
    sal_uLong       GetColOffset( SCCOL nCol, bool bHiddenAsZero = true ) const;
    sal_uLong       GetRowOffset( SCROW nRow, bool bHiddenAsZero = true ) const;

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

    sal_uInt16      GetCommonWidth( SCCOL nEndCol ) const;

    SCROW       GetHiddenRowCount( SCROW nRow ) const;

    void        ShowCol(SCCOL nCol, bool bShow);
    void        ShowRow(SCROW nRow, bool bShow);
    void        DBShowRow(SCROW nRow, bool bShow);

    void        ShowRows(SCROW nRow1, SCROW nRow2, bool bShow);
    void        DBShowRows(SCROW nRow1, SCROW nRow2, bool bShow);

    void        SetRowFlags( SCROW nRow, CRFlags nNewFlags );
    void        SetRowFlags( SCROW nStartRow, SCROW nEndRow, CRFlags nNewFlags );

                /// @return  the index of the last row with any set flags (auto-pagebreak is ignored).
    SCROW      GetLastFlaggedRow() const;

                /// @return  the index of the last changed column (flags and column width, auto pagebreak is ignored).
    SCCOL      GetLastChangedCol() const;
                /// @return  the index of the last changed row (flags and row height, auto pagebreak is ignored).
    SCROW      GetLastChangedRow() const;

    bool       IsDataFiltered(SCCOL nColStart, SCROW nRowStart, SCCOL nColEnd, SCROW nRowEnd) const;
    bool       IsDataFiltered(const ScRange& rRange) const;
    CRFlags    GetColFlags( SCCOL nCol ) const;
    CRFlags    GetRowFlags( SCROW nRow ) const;

    const ScBitMaskCompressedArray< SCROW, CRFlags> * GetRowFlagsArray() const
                    { return pRowFlags; }

    bool        UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, bool bShow );
    bool        UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, bool bShow );

    void        UpdatePageBreaks( const ScRange* pUserArea );
    void        RemoveManualBreaks();
    bool        HasManualBreaks() const;
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
    css::uno::Sequence<
        css::sheet::TablePageBreakData> GetRowBreakData() const;

    bool        RowHidden(SCROW nRow, SCROW* pFirstRow = nullptr, SCROW* pLastRow = nullptr) const;
    bool        RowHiddenLeaf(SCROW nRow, SCROW* pFirstRow = nullptr, SCROW* pLastRow = nullptr) const;
    bool        HasHiddenRows(SCROW nStartRow, SCROW nEndRow) const;
    bool        ColHidden(SCCOL nCol, SCCOL* pFirstCol = nullptr, SCCOL* pLastCol = nullptr) const;
    bool        SetRowHidden(SCROW nStartRow, SCROW nEndRow, bool bHidden);
    void        SetColHidden(SCCOL nStartCol, SCCOL nEndCol, bool bHidden);
    void        CopyColHidden(ScTable& rTable, SCCOL nStartCol, SCCOL nEndCol);
    void        CopyRowHidden(ScTable& rTable, SCROW nStartRow, SCROW nEndRow);
    void        CopyRowHeight(ScTable& rSrcTable, SCROW nStartRow, SCROW nEndRow, SCROW nSrcOffset);
    SCROW       FirstVisibleRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW       LastVisibleRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW       CountVisibleRows(SCROW nStartRow, SCROW nEndRow) const;
    sal_uInt32  GetTotalRowHeight(SCROW nStartRow, SCROW nEndRow, bool bHiddenAsZero = true) const;

    SCCOLROW    LastHiddenColRow(SCCOLROW nPos, bool bCol) const;

    bool        RowFiltered(SCROW nRow, SCROW* pFirstRow = nullptr, SCROW* pLastRow = nullptr) const;
    bool        ColFiltered(SCCOL nCol, SCCOL* pFirstCol = nullptr, SCCOL* pLastCol = nullptr) const;
    bool        HasFilteredRows(SCROW nStartRow, SCROW nEndRow) const;
    void        CopyColFiltered(ScTable& rTable, SCCOL nStartCol, SCCOL nEndCol);
    void        CopyRowFiltered(ScTable& rTable, SCROW nStartRow, SCROW nEndRow);
    void        SetRowFiltered(SCROW nStartRow, SCROW nEndRow, bool bFiltered);
    void        SetColFiltered(SCCOL nStartCol, SCCOL nEndCol, bool bFiltered);
    SCROW       FirstNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW       LastNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW       CountNonFilteredRows(SCROW nStartRow, SCROW nEndRow) const;

    bool IsManualRowHeight(SCROW nRow) const;

    bool HasUniformRowHeight( SCROW nRow1, SCROW nRow2 ) const;

    void        SyncColRowFlags();

    void        StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );
    void        ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );

    /** Sort a range of data. */
    void Sort(
        const ScSortParam& rSortParam, bool bKeepQuery, bool bUpdateRefs,
        ScProgress* pProgress, sc::ReorderParam* pUndo );

    void Reorder( const sc::ReorderParam& rParam );

    bool ValidQuery(
        SCROW nRow, const ScQueryParam& rQueryParam, ScRefCellValue* pCell = nullptr,
        bool* pbTestEqualCondition = nullptr);
    void        TopTenQuery( ScQueryParam& );
    SCSIZE      Query(ScQueryParam& rQueryParam, bool bKeepSub);
    bool        CreateQueryParam(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);

    void GetFilterEntries(SCCOL nCol, SCROW nRow1, SCROW nRow2, std::vector<ScTypedStrData>& rStrings, bool& rHasDates);
    void GetFilteredFilterEntries(SCCOL nCol, SCROW nRow1, SCROW nRow2, const ScQueryParam& rParam, std::vector<ScTypedStrData>& rStrings, bool& rHasDates );
    bool GetDataEntries(SCCOL nCol, SCROW nRow, std::set<ScTypedStrData>& rStrings, bool bLimit);

    bool        HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow ) const;
    bool        HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow ) const;

    sal_Int32   GetMaxStringLen( SCCOL nCol,
                                    SCROW nRowStart, SCROW nRowEnd, rtl_TextEncoding eCharSet ) const;
    sal_Int32  GetMaxNumberStringLen( sal_uInt16& nPrecision,
                                       SCCOL nCol,
                                       SCROW nRowStart, SCROW nRowEnd ) const;

    bool        IsSortCollatorGlobal() const;
    void        InitSortCollator( const ScSortParam& rPar );
    void        DestroySortCollator();
    void        SetDrawPageSize( bool bResetStreamValid = true, bool bUpdateNoteCaptionPos = true );

    void SetRangeName(ScRangeName* pNew);
    ScRangeName* GetRangeName() const;

    void PreprocessRangeNameUpdate(
        sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt );

    void CompileHybridFormula(
        sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt );

    void PreprocessDBDataUpdate(
        sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt );

    ScConditionalFormatList* GetCondFormList();
    const ScConditionalFormatList* GetCondFormList() const;
    void SetCondFormList( ScConditionalFormatList* pList );

    void DeleteConditionalFormat(sal_uLong nOldIndex);

    sal_uLong          AddCondFormat( ScConditionalFormat* pNew );

    SvtScriptType GetScriptType( SCCOL nCol, SCROW nRow ) const;
    void SetScriptType( SCCOL nCol, SCROW nRow, SvtScriptType nType );
    void UpdateScriptTypes( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    SvtScriptType GetRangeScriptType( sc::ColumnBlockPosition& rBlockPos, SCCOL nCol, SCROW nRow1, SCROW nRow2 );

    size_t GetFormulaHash( SCCOL nCol, SCROW nRow ) const;

    ScFormulaVectorState GetFormulaVectorState( SCCOL nCol, SCROW nRow ) const;
    formula::FormulaTokenRef ResolveStaticReference( SCCOL nCol, SCROW nRow );
    formula::FormulaTokenRef ResolveStaticReference( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    formula::VectorRefArray FetchVectorRefArray( SCCOL nCol, SCROW nRow1, SCROW nRow2 );

    void SplitFormulaGroups( SCCOL nCol, std::vector<SCROW>& rRows );
    void UnshareFormulaCells( SCCOL nCol, std::vector<SCROW>& rRows );
    void RegroupFormulaCells( SCCOL nCol );

    ScRefCellValue GetRefCellValue( SCCOL nCol, SCROW nRow );

    SvtBroadcaster* GetBroadcaster( SCCOL nCol, SCROW nRow );
    const SvtBroadcaster* GetBroadcaster( SCCOL nCol, SCROW nRow ) const;
    void DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, SCCOL nCol, SCROW nRow1, SCROW nRow2 );

    void FillMatrix( ScMatrix& rMat, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, svl::SharedStringPool* pPool ) const;

    void InterpretDirtyCells( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    void SetFormulaResults( SCCOL nCol, SCROW nRow, const double* pResults, size_t nLen );
    void SetFormulaResults( SCCOL nCol, SCROW nRow, const formula::FormulaConstTokenRef* pResults, size_t nLen );

    /**
     * Either start all formula cells as listeners unconditionally, or start
     * those that are marked "needs listening".
     *
     * @param rCxt context object.
     * @param bAll when true, start all formula cells as listeners. When
     *             false, only start those that are marked "needs listening".
     */
    void StartListeners( sc::StartListeningContext& rCxt, bool bAll );

    /**
     * Mark formula cells dirty that have the mbPostponedDirty flag set or
     * contain named ranges with relative references.
     */
    void SetDirtyIfPostponed();

    /**
     * Broadcast dirty formula cells that contain functions such as CELL(),
     * COLUMN() or ROW() which may change its value on move.
     */
    void BroadcastRecalcOnRefMove();

    void CollectListeners( std::vector<SvtListener*>& rListeners, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    void TransferListeners(
        ScTable& rDestTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        SCCOL nColDelta, SCROW nRowDelta );

    void TransferCellValuesTo( SCCOL nCol, SCROW nRow, size_t nLen, sc::CellValues& rDest );
    void CopyCellValuesFrom( SCCOL nCol, SCROW nRow, const sc::CellValues& rSrc );

    void ConvertFormulaToValue(
        sc::EndListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        sc::TableValues* pUndo );

    void SwapNonEmpty(
        sc::TableValues& rValues, sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt );

    void finalizeOutlineImport();

#if DUMP_COLUMN_STORAGE
    void DumpColumnStorage( SCCOL nCol ) const;
#endif

    /** Replace behaves differently to the Search; adjust the rCol and rRow accordingly.

        'Replace' replaces at the 'current' position, but in order to achieve
        that, we have to 'shift' the rCol / rRow to the 'previous' position -
        what it is depends on various settings in rSearchItem.
    */
    static void UpdateSearchItemAddressForReplace( const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow );

private:

    void FillFormulaVertical(
        const ScFormulaCell& rSrcCell,
        SCCOLROW& rInner, SCCOL nCol, SCROW nRow1, SCROW nRow2,
        ScProgress* pProgress, sal_uLong& rProgress );

    void FillSeriesSimple(
        ScCellValue& rSrcCell, SCCOLROW& rInner, SCCOLROW nIMin, SCCOLROW nIMax,
        SCCOLROW& rCol, SCCOLROW& rRow, bool bVertical, ScProgress* pProgress, sal_uLong& rProgress );

    void FillAutoSimple(
        SCCOLROW nISrcStart, SCCOLROW nISrcEnd, SCCOLROW nIStart, SCCOLROW nIEnd,
        SCCOLROW& rInner, SCCOLROW& rCol, SCCOLROW& rRow,
        sal_uLong nActFormCnt, sal_uLong nMaxFormCnt,
        bool bHasFiltered, bool bVertical, bool bPositive,
        ScProgress* pProgress, sal_uLong& rProgress );

    void        FillSeries( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd,
                                FillDateCmd eFillDateCmd,
                                double nStepValue, double nMaxValue, sal_uInt16 nMinDigits,
                                bool bAttribs, ScProgress* pProgress );
    void        FillAnalyse( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                FillCmd& rCmd, FillDateCmd& rDateCmd,
                                double& rInc, sal_uInt16& rMinDigits,
                                ScUserListData*& rListData, sal_uInt16& rListIndex);
    void        FillAuto( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        sal_uLong nFillCount, FillDir eFillDir, ScProgress* pProgress );

    bool        ValidNextPos( SCCOL nCol, SCROW nRow, const ScMarkData& rMark,
                                bool bMarked, bool bUnprotected ) const;

    void        AutoFormatArea(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                const ScPatternAttr& rAttr, sal_uInt16 nFormatNo);
    void        GetAutoFormatAttr(SCCOL nCol, SCROW nRow, sal_uInt16 nIndex, ScAutoFormatData& rData);
    void        GetAutoFormatFrame(SCCOL nCol, SCROW nRow, sal_uInt16 nFlags, sal_uInt16 nIndex, ScAutoFormatData& rData);
    bool        SearchCell(const SvxSearchItem& rSearchItem, SCCOL nCol, SCROW nRow,
                           const ScMarkData& rMark, OUString& rUndoStr, ScDocument* pUndoDoc);
    bool        Search(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                       const ScMarkData& rMark, OUString& rUndoStr, ScDocument* pUndoDoc);
    bool        Search(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                       SCCOL nLastCol, SCROW nLastRow,
                       const ScMarkData& rMark, OUString& rUndoStr, ScDocument* pUndoDoc);
    bool        SearchAll(const SvxSearchItem& rSearchItem, const ScMarkData& rMark,
                          ScRangeList& rMatchedRanges, OUString& rUndoStr, ScDocument* pUndoDoc);
    bool        Replace(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                        const ScMarkData& rMark, OUString& rUndoStr, ScDocument* pUndoDoc);
    bool        ReplaceAll(
        const SvxSearchItem& rSearchItem, const ScMarkData& rMark, ScRangeList& rMatchedRanges,
        OUString& rUndoStr, ScDocument* pUndoDoc);

    bool        SearchStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                            const ScMarkData& rMark);
    bool        ReplaceStyle(const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                             const ScMarkData& rMark, bool bIsUndo);
    bool        SearchAllStyle(
        const SvxSearchItem& rSearchItem, const ScMarkData& rMark, ScRangeList& rMatchedRanges);
    bool        ReplaceAllStyle(
        const SvxSearchItem& rSearchItem, const ScMarkData& rMark, ScRangeList& rMatchedRanges,
        ScDocument* pUndoDoc);
    bool        SearchAndReplaceEmptyCells(
                    const SvxSearchItem& rSearchItem,
                    SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark, ScRangeList& rMatchedRanges,
                    OUString& rUndoStr, ScDocument* pUndoDoc);
    bool        SearchRangeForEmptyCell(const ScRange& rRange,
                    const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow,
                    OUString& rUndoStr);
    bool        SearchRangeForAllEmptyCells(
        const ScRange& rRange, const SvxSearchItem& rSearchItem,
        ScRangeList& rMatchedRanges, OUString& rUndoStr, ScDocument* pUndoDoc);

                                // use the global sort parameter:
    bool        IsSorted(SCCOLROW nStart, SCCOLROW nEnd) const;
    static void DecoladeRow( ScSortInfoArray*, SCROW nRow1, SCROW nRow2 );
    short CompareCell(
        sal_uInt16 nSort,
        ScRefCellValue& rCell1, SCCOL nCell1Col, SCROW nCell1Row,
        ScRefCellValue& rCell2, SCCOL nCell2Col, SCROW nCell2Row ) const;
    short       Compare(SCCOLROW nIndex1, SCCOLROW nIndex2) const;
    short       Compare( ScSortInfoArray*, SCCOLROW nIndex1, SCCOLROW nIndex2) const;
    ScSortInfoArray* CreateSortInfoArray( const sc::ReorderParam& rParam );
    ScSortInfoArray* CreateSortInfoArray(
        const ScSortParam& rSortParam, SCCOLROW nInd1, SCCOLROW nInd2,
        bool bKeepQuery, bool bUpdateRefs );
    void        QuickSort( ScSortInfoArray*, SCsCOLROW nLo, SCsCOLROW nHi);
    void SortReorderByColumn( ScSortInfoArray* pArray, SCROW nRow1, SCROW nRow2, bool bPattern, ScProgress* pProgress );

    void SortReorderByRow( ScSortInfoArray* pArray, SCCOL nCol1, SCCOL nCol2, ScProgress* pProgress );
    void SortReorderByRowRefUpdate( ScSortInfoArray* pArray, SCCOL nCol1, SCCOL nCol2, ScProgress* pProgress );

    bool        CreateExcelQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);
    bool        CreateStarQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);
    void        GetUpperCellString(SCCOL nCol, SCROW nRow, OUString& rStr);

    bool        RefVisible(ScFormulaCell* pCell);

    bool        IsEmptyLine(SCROW nRow, SCCOL nStartCol, SCCOL nEndCol) const;

    void        IncDate(double& rVal, sal_uInt16& nDayOfMonth, double nStep, FillDateCmd eCmd);
    void FillFormula(
        ScFormulaCell* pSrcCell, SCCOL nDestCol, SCROW nDestRow, bool bLast );
    void        UpdateInsertTabAbs(SCTAB nNewPos);
    bool        GetNextSpellingCell(SCCOL& rCol, SCROW& rRow, bool bInSel,
                                    const ScMarkData& rMark) const;
    bool        GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark ) const;
    void        TestTabRefAbs(SCTAB nTable) const;
    void CompileDBFormula( sc::CompileFormulaContext& rCxt );
    void CompileColRowNameFormula( sc::CompileFormulaContext& rCxt );

    void        StartListening( const ScAddress& rAddress, SvtListener* pListener );
    void        EndListening( const ScAddress& rAddress, SvtListener* pListener );
    void StartListening( sc::StartListeningContext& rCxt, const ScAddress& rAddress, SvtListener& rListener );
    void EndListening( sc::EndListeningContext& rCxt, const ScAddress& rAddress, SvtListener& rListener );

    void AttachFormulaCells( sc::StartListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    void DetachFormulaCells( sc::EndListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    void        SetLoadingMedium(bool bLoading);

    SCSIZE      FillMaxRot( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2,
                            SCCOL nCol, SCROW nAttrRow1, SCROW nAttrRow2, SCSIZE nArrY,
                            const ScPatternAttr* pPattern, const SfxItemSet* pCondSet );

    // idle calculation of OutputDevice text width for cell
    // also invalidates script type, broadcasts for "calc as shown"
    void        InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo,
                                     bool bNumFormatChanged, bool bBroadcast );

    void        SkipFilteredRows(SCROW& rRow, SCROW& rLastNonFilteredRow, bool bForward);

    /**
     * In case the cell text goes beyond the column width, move the max column
     * position to the right.  This is called from ExtendPrintArea.
     */
    void        MaybeAddExtraColumn(SCCOL& rCol, SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY);

    void        CopyPrintRange(const ScTable& rTable);

    SCCOL       FindNextVisibleColWithContent(SCCOL nCol, bool bRight, SCROW nRow) const;

    SCCOL       FindNextVisibleCol(SCCOL nCol, bool bRight) const;

    // Clipboard transpose for notes
    void TransposeColNotes(ScTable* pTransClip, SCCOL nCol1, SCCOL nCol, SCROW nRow1, SCROW nRow2);

    ScColumn* FetchColumn( SCCOL nCol );
    const ScColumn* FetchColumn( SCCOL nCol ) const;

    void EndListeningIntersectedGroup(
        sc::EndListeningContext& rCxt, SCCOL nCol, SCROW nRow, std::vector<ScAddress>* pGroupPos );

    void EndListeningIntersectedGroups(
        sc::EndListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        std::vector<ScAddress>* pGroupPos );

    void EndListeningGroup( sc::EndListeningContext& rCxt, SCCOL nCol, SCROW nRow );
    void SetNeedsListeningGroup( SCCOL nCol, SCROW nRow );

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
        ScRefCellValue reset(SCROW nRow);

        /**
         * Find the next visible data cell position.
         *
         * @return Next visible data cell if found, or NULL otherwise.
         */
        ScRefCellValue next();

        /**
         * Get the current row position.
         *
         * @return Current row position, or ROW_NOT_FOUND if the iterator
         *         doesn't point to a valid data cell position.
         */
        SCROW getRow() const { return mnCurRow;}

    private:
        ScFlatBoolRowSegments& mrRowSegs;
        ScColumn& mrColumn;
        ScRefCellValue maCell;
        SCROW mnCurRow;
        SCROW mnUBound;
    };

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
