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

#pragma once

#include "global.hxx"
#include "address.hxx"
#include "cellvalue.hxx"
#include "columnspanset.hxx"
#include "rangelst.hxx"
#include "types.hxx"
#include "mtvelements.hxx"
#include <formula/types.hxx>
#include <svl/zforlist.hxx>
#include <svx/svdobj.hxx>
#include "attarray.hxx"

#include <optional>
#include <set>
#include <vector>

#include <mdds/flat_segment_tree.hpp>

namespace editeng { class SvxBorderLine; }
namespace formula { struct VectorRefArray; }

namespace sc {

struct BroadcasterState;
struct FormulaGroupEntry;
class StartListeningContext;
class EndListeningContext;
class CopyFromClipContext;
class CopyToClipContext;
class CopyToDocContext;
class MixDocContext;
struct RefUpdateContext;
struct RefUpdateInsertTabContext;
struct RefUpdateDeleteTabContext;
struct RefUpdateMoveTabContext;
class EditTextIterator;
struct NoteEntry;
class DocumentStreamAccess;
class CellValues;
class TableValues;
struct RowSpan;
class RowHeightContext;
class CompileFormulaContext;
struct SetFormulaDirtyContext;
enum class MatrixEdge;
class ColumnIterator;
class Sparkline;

}

class Fraction;
class OutputDevice;
class ScItemPoolCache;
class SvtListener;
class SfxPoolItem;
class SfxStyleSheetBase;
class SvxBoxInfoItem;
class SvxBoxItem;

class ScDocument;
class ScEditDataArray;
class ScFormulaCell;
class ScMarkData;
class ScPatternAttr;
class ScStyleSheet;
class SvtBroadcaster;
class ScTypedStrData;
class ScProgress;
class ScFunctionData;
class ScFlatBoolRowSegments;
struct ScSetStringParam;
struct ScColWidthParam;
struct ScRefCellValue;
struct ScCellValue;
class ScHint;
enum class ScMF;
struct ScFilterEntries;
struct ScInterpreterContext;

struct ScNeededSizeOptions
{
    CellAttributeHolder aPattern;

    bool                bFormula;
    bool                bSkipMerged;
    bool                bGetFont;
    bool                bTotalSize;

    ScNeededSizeOptions();
};

// A shared implementation of some column code and data.
// This is used by ScColumn, but also by ScTable for the unallocated
// columns (one instance that is the default for all not-yet allocated columns).
class ScColumnData
{
protected:
    std::unique_ptr<ScAttrArray> pAttrArray;

    ScColumnData(const ScColumnData&) = delete;
    ScColumnData& operator= (const ScColumnData&) = delete;

public:
    ScColumnData() = default;
    void InitAttrArray(ScAttrArray* attrArray) { pAttrArray.reset(attrArray); }

    ScDocument& GetDoc() const { return pAttrArray->GetDoc(); }

    ScAttrArray& AttrArray() { return *pAttrArray; }
    const ScAttrArray& AttrArray() const { return *pAttrArray; }

    const SfxPoolItem&      GetAttr( SCROW nRow, sal_uInt16 nWhich ) const;
    template<class T> const T&  GetAttr( SCROW nRow, TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T&>(GetAttr(nRow, sal_uInt16(nWhich)));
    }
    const SfxPoolItem&      GetAttr( SCROW nRow, sal_uInt16 nWhich, SCROW& nStartRow, SCROW& nEndRow ) const;
    template<class T> const T&  GetAttr( SCROW nRow, TypedWhichId<T> nWhich, SCROW& nStartRow, SCROW& nEndRow ) const
    {
        return static_cast<const T&>(GetAttr(nRow, sal_uInt16(nWhich), nStartRow, nEndRow));
    }

    void        SetAttrEntries(std::vector<ScAttrEntry> && vNewData);

    const ScPatternAttr*    GetPattern( SCROW nRow ) const;
    const ScPatternAttr*    GetMostUsedPattern( SCROW nStartRow, SCROW nEndRow ) const;
    void        ApplySelectionStyle(const ScStyleSheet& rStyle, SCROW nTop, SCROW nBottom);
    void        ApplySelectionCache(ScItemPoolCache& rCache, SCROW nStartRow, SCROW nEndRow,
                                    ScEditDataArray* pDataArray, bool* pIsChanged);
    void        ApplyPatternArea( SCROW nStartRow, SCROW nEndRow, const ScPatternAttr& rPatAttr,
                                  ScEditDataArray* pDataArray = nullptr,
                                  bool* const pIsChanged = nullptr);
    void        MergePatternArea( ScMergePatternState& rState, SCROW nRow1, SCROW nRow2, bool bDeep ) const;

    sal_uInt32  GetNumberFormat( const ScInterpreterContext& rContext, SCROW nRow ) const;
    sal_uInt32  GetNumberFormat( SCROW nStartRow, SCROW nEndRow ) const;

    /// Including current, may return -1
    SCROW       GetNextUnprotected( SCROW nRow, bool bUp ) const;

    const ScStyleSheet* GetStyle( SCROW nRow ) const;
    void        ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, const ScStyleSheet& rStyle );

    bool        HasAttrib( SCROW nRow1, SCROW nRow2, HasAttrFlags nMask ) const;
    bool        HasAttrib( SCROW nRow, HasAttrFlags nMask, SCROW* nStartRow = nullptr, SCROW* nEndRow = nullptr ) const;

    std::unique_ptr<ScAttrIterator> CreateAttrIterator( SCROW nStartRow, SCROW nEndRow ) const;

    bool        IsAllAttrEqual( const ScColumnData& rCol, SCROW nStartRow, SCROW nEndRow ) const;

    void        ClearSelectionItems(const sal_uInt16* pWhich, SCROW nStartRow, SCROW nEndRow);
    void        ChangeSelectionIndent(bool bIncrement, SCROW nStartRow, SCROW nEndRow);

    bool        TestInsertRow( SCSIZE nSize ) const;
    void        InsertRow( SCROW nStartRow, SCSIZE nSize );
    void        DeleteRow( SCROW nStartRow, SCSIZE nSize );

    // Applies a function to the selected ranges.
    // The function looks like
    //     ApplyDataFunc(ScColumnData& applyTo, SCROW nTop, SCROW nBottom)
    template <typename ApplyDataFunc> void Apply(const ScMarkData&, SCCOL, ApplyDataFunc);
};

// Use protected inheritance to prevent publishing some internal ScColumnData
// functions as part of ScColumn. If they should be public in ScColumn,
// use 'using' to make them public.
class ScColumn : protected ScColumnData
{
    // Empty values correspond with empty cells. All non-empty cell positions
    // must have non-empty elements. For text width, the value should be
    // either the real text width, or TEXTWIDTH_DIRTY in case it hasn't been
    // calculated yet. For script type, it should be either the real script
    // type value or SvtScriptType::UNKNOWN.
    sc::CellTextAttrStoreType maCellTextAttrs;

    // Cell notes
    sc::CellNoteStoreType maCellNotes;

    // Broadcasters for formula cells.
    sc::BroadcasterStoreType maBroadcasters;

    // Cell values.
    sc::CellStoreType maCells;

    // Sparklines
    sc::SparklineStoreType maSparklines;

    sal_uInt32 mnBlkCountFormula;
    sal_uInt32 mnBlkCountCellNotes;

    SCCOL           nCol;
    SCTAB           nTab;

    bool mbEmptyBroadcastersPending : 1; // a broadcaster not removed during EnableDelayDeletingBroadcasters()

friend class ScDocument;                    // for FillInfo
friend class ScTable;
friend class ScValueIterator;
friend class ScHorizontalValueIterator;
friend class ScDBQueryDataIterator;
template< ScQueryCellIteratorAccess accessType, ScQueryCellIteratorType queryType >
friend class ScQueryCellIteratorBase;
template< ScQueryCellIteratorAccess accessType >
friend class ScQueryCellIteratorAccessSpecific;
friend class ScFormulaGroupIterator;
friend class ScCellIterator;
friend class ScHorizontalCellIterator;
friend class ScColumnTextWidthIterator;
friend class ScDocumentImport;
friend class sc::DocumentStreamAccess;
friend class sc::SingleColumnSpanSet;
friend class sc::ColumnSpanSet;
friend class sc::EditTextIterator;
friend class sc::CellValues;
friend class sc::TableValues;
friend class sc::CellStoreEvent;

    bool ParseString(
        ScCellValue& rCell,
        SCROW nRow, SCTAB nTab, const OUString& rString, formula::FormulaGrammar::AddressConvention eConv,
        const ScSetStringParam* pParam );

    void duplicateSparkline(sc::CopyFromClipContext& rContext, sc::ColumnBlockPosition* pBlockPos,
                            size_t nColOffset, size_t nDestSize, ScAddress aDestPosition);

public:

    /** Broadcast mode for SetDirty(SCROW,SCROW,BroadcastMode). */
    enum BroadcastMode
    {
        BROADCAST_NONE,             ///< no broadcasting
        BROADCAST_DATA_POSITIONS,   ///< broadcast existing cells with position => does AreaBroadcast
        BROADCAST_BROADCASTERS      ///< broadcast only existing cell broadcasters => no AreaBroadcast of range!
    };

    ScColumn(ScSheetLimits const &);
    ~ScColumn() COVERITY_NOEXCEPT_FALSE;

    void        Init(SCCOL nNewCol, SCTAB nNewTab, ScDocument& rDoc, bool bEmptyAttrArray);

    using ScColumnData::GetDoc;
    SCTAB GetTab() const { return nTab; }
    SCCOL GetCol() const { return nCol; }
    sc::CellStoreType& GetCellStore() { return maCells; }
    const sc::CellStoreType& GetCellStore() const { return maCells; }
    sc::CellTextAttrStoreType& GetCellAttrStore() { return maCellTextAttrs; }
    const sc::CellTextAttrStoreType& GetCellAttrStore() const { return maCellTextAttrs; }
    sc::CellNoteStoreType& GetCellNoteStore() { return maCellNotes; }
    const sc::CellNoteStoreType& GetCellNoteStore() const { return maCellNotes; }
    sc::SparklineStoreType& GetSparklineStore() { return maSparklines; }
    const sc::SparklineStoreType& GetSparklineStore() const { return maSparklines; }

    ScRefCellValue GetCellValue( SCROW nRow ) const;
    ScRefCellValue GetCellValue( sc::ColumnBlockPosition& rBlockPos, SCROW nRow );
    ScRefCellValue GetCellValue( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow ) const;
    static ScRefCellValue GetCellValue( const sc::CellStoreType::const_iterator& itPos, size_t nOffset );

    const sc::CellTextAttr* GetCellTextAttr( SCROW nRow ) const;
    const sc::CellTextAttr* GetCellTextAttr( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow ) const;

    void        Delete( SCROW nRow );
    void        DeleteContent( SCROW nRow, bool bBroadcast = true );
    void        FreeAll();
    void        FreeNotes();
    void        Swap( ScColumn& rOther, SCROW nRow1, SCROW nRow2, bool bPattern );

    bool        HasAttribSelection( const ScMarkData& rMark, HasAttrFlags nMask ) const;
    bool        IsMerged( SCROW nRow ) const;
    bool        ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                bool bRefresh );

    bool        IsEmptyData() const;
    bool        IsEmptyAttr() const;

                // data only:
    bool        IsEmptyData(SCROW nStartRow, SCROW nEndRow) const;
    SCSIZE      GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const;
    bool        HasDataAt( SCROW nRow, ScDataAreaExtras* pDataAreaExtras = nullptr ) const;
    bool        HasDataAt( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow,
                           ScDataAreaExtras* pDataAreaExtras = nullptr ) const;
    bool        HasDataAt( sc::ColumnBlockPosition& rBlockPos, SCROW nRow,
                           ScDataAreaExtras* pDataAreaExtras = nullptr );
    void        GetDataExtrasAt( SCROW nRow, ScDataAreaExtras& rDataAreaExtras ) const;
    bool        HasVisibleDataAt(SCROW nRow) const;
    SCROW       GetFirstDataPos() const;
    SCROW       GetLastDataPos() const;
    SCROW       GetLastDataPos( SCROW nLastRow, ScDataAreaExtras* pDataAreaExtras = nullptr ) const;
    bool        GetPrevDataPos(SCROW& rRow) const;
    bool        GetNextDataPos(SCROW& rRow) const;
    bool        TrimEmptyBlocks(SCROW& rRowStart, SCROW& rRowEnd) const;
    void        FindDataAreaPos(SCROW& rRow, bool bDown) const; // (without Broadcaster)
    void        FindUsed( SCROW nStartRow, SCROW nEndRow, mdds::flat_segment_tree<SCROW, bool>& rUsed ) const;

    SCSIZE             VisibleCount( SCROW nStartRow, SCROW nEndRow ) const;
    sc::MatrixEdge     GetBlockMatrixEdges(SCROW nRow1, SCROW nRow2, sc::MatrixEdge nMask, bool bNoMatrixAtAll ) const;
    // Repeated calls to HasSelectionMatrixFragment() repeatedly call rMark.GetMarkedRanges(),
    // which may be quite slow. For that reason first save the result of rMark.GetMarkedRanges()
    // pass that to HasSelectionMatrixFragment() calls.
    bool    HasSelectionMatrixFragment(const ScMarkData& rMark, const ScRangeList& rRangeList) const;

    bool    GetFirstVisibleAttr( SCROW& rFirstRow ) const;
    bool    GetLastVisibleAttr( SCROW& rLastRow, bool bSkipEmpty ) const;
    bool    HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const;
    bool    IsVisibleAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const;

    bool    TestInsertCol( SCROW nStartRow, SCROW nEndRow) const;
    bool TestInsertRow( SCROW nStartRow, SCSIZE nSize ) const;
    void        InsertRow( SCROW nStartRow, SCSIZE nSize );
    void GetUnprotectedCells(SCROW nStartRow, SCROW nEndRow, ScRangeList& rRangeList ) const;

    /**
     * @param nStartRow top row position
     * @param nSize size of the segment to delete.
     * @param pGroupPos when non-NULL, stores the top position of formula
     *                  group that's been merged as a result of row deletion.
     */
    void DeleteRow( SCROW nStartRow, SCSIZE nSize, std::vector<ScAddress>* pGroupPos );

    void DeleteArea(
        SCROW nStartRow, SCROW nEndRow, InsertDeleteFlags nDelFlag,
        bool bBroadcast = true, sc::ColumnSpanSet* pBroadcastSpans = nullptr );

    void DeleteRanges( const std::vector<sc::RowSpan>& rRanges, InsertDeleteFlags nDelFlag );

    void CopyToClip(
        sc::CopyToClipContext& rCxt, SCROW nRow1, SCROW nRow2, ScColumn& rColumn ) const;

    void CopyStaticToDocument(
        SCROW nRow1, SCROW nRow2, const SvNumberFormatterMergeMap& rMap, ScColumn& rDestCol );

    void CopyCellToDocument( SCROW nSrcRow, SCROW nDestRow, ScColumn& rDestCol );
    void InitBlockPosition( sc::ColumnBlockPosition& rBlockPos );
    void InitBlockPosition( sc::ColumnBlockConstPosition& rBlockPos ) const;

    void DeleteBeforeCopyFromClip(
        sc::CopyFromClipContext& rCxt, const ScColumn& rClipCol, sc::ColumnSpanSet& rBroadcastSpans );

    void CopyOneCellFromClip( sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2, size_t nColOffset );

    void CopyFromClip(
        sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2, tools::Long nDy, ScColumn& rColumn );

    void RemoveEditAttribs( sc::ColumnBlockPosition& rBlockPos, SCROW nStartRow, SCROW nEndRow );

                //  Selection (?) of this document
    void MixMarked(
        sc::MixDocContext& rCxt, const ScMarkData& rMark, ScPasteFunc nFunction,
        bool bSkipEmpty, const ScColumn& rSrcCol );
    void MixData(
        sc::MixDocContext& rCxt, SCROW nRow1, SCROW nRow2, ScPasteFunc nFunction, bool bSkipEmpty,
        const ScColumn& rSrcCol );

    void UpdateSelectionFunction(
        const ScRangeList& rRanges, ScFunctionData& rData, const ScFlatBoolRowSegments& rHiddenRows );

    void CopyToColumn(
        sc::CopyToDocContext& rCxt, SCROW nRow1, SCROW nRow2, InsertDeleteFlags nFlags, bool bMarked,
        ScColumn& rColumn, const ScMarkData* pMarkData = nullptr, bool bAsLink = false,
        bool bGlobalNamesToLocal = false ) const;

    void UndoToColumn(
        sc::CopyToDocContext& rCxt, SCROW nRow1, SCROW nRow2, InsertDeleteFlags nFlags, bool bMarked,
        ScColumn& rColumn) const;

    void        CopyScenarioFrom( const ScColumn& rSrcCol );
    void        CopyScenarioTo( ScColumn& rDestCol ) const;
    bool       TestCopyScenarioTo( const ScColumn& rDestCol ) const;
    void        MarkScenarioIn( ScMarkData& rDestMark ) const;

    void        CopyUpdated( const ScColumn* pPosCol, ScColumn& rDestCol ) const;

    void        SwapCol(ScColumn& rCol);
    void        MoveTo(SCROW nStartRow, SCROW nEndRow, ScColumn& rCol);

    bool HasEditCells(SCROW nStartRow, SCROW nEndRow, SCROW& rFirst);

    bool SetString(
        SCROW nRow, SCTAB nTab, const OUString& rString, formula::FormulaGrammar::AddressConvention eConv,
        const ScSetStringParam* pParam = nullptr );

    void SetEditText( SCROW nRow, std::unique_ptr<EditTextObject> pEditText );
    void SetEditText( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, std::unique_ptr<EditTextObject> pEditText );
    void SetEditText( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, const EditTextObject& rEditText );
    void SetEditText( SCROW nRow, const EditTextObject& rEditText, const SfxItemPool* pEditPool );
    void SetFormula( SCROW nRow, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram );
    void SetFormula( SCROW nRow, const OUString& rFormula, formula::FormulaGrammar::Grammar eGram );

    /**
     * Takes ownership of pCell
     *
     * @return pCell if it was successfully inserted, NULL otherwise. pCell
     *         is deleted automatically on failure to insert.
     */
    ScFormulaCell* SetFormulaCell(
        SCROW nRow, ScFormulaCell* pCell,
        sc::StartListeningType eListenType = sc::SingleCellListening,
        bool bInheritNumFormatIfNeeded = true);
    void SetFormulaCell(
        sc::ColumnBlockPosition& rBlockPos, SCROW nRow, ScFormulaCell* pCell,
        sc::StartListeningType eListenType = sc::SingleCellListening,
        bool bInheritNumFormatIfNeeded = true);

    bool SetFormulaCells( SCROW nRow, std::vector<ScFormulaCell*>& rCells );

    bool HasFormulaCell() const;
    bool HasFormulaCell( SCROW nRow1, SCROW nRow2 ) const;

    void CloneFormulaCell(
        sc::ColumnBlockPosition& rBlockPos,
        const ScFormulaCell& rSrc, const sc::CellTextAttr& rAttr,
        const std::vector<sc::RowSpan>& rRanges );

    void CloneFormulaCell(
        const ScFormulaCell& rSrc, const sc::CellTextAttr& rAttr,
        const std::vector<sc::RowSpan>& rRanges );

    svl::SharedString GetSharedString( SCROW nRow ) const;

    void SetRawString( SCROW nRow, const OUString& rStr );
    void SetRawString( SCROW nRow, const svl::SharedString& rStr );
    void SetRawString( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, const svl::SharedString& rStr, bool bBroadcast = true );
    void SetValue( SCROW nRow, double fVal );
    void SetValues( const SCROW nRow, const std::vector<double>& rVals );
    void SetValue( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, double fVal, bool bBroadcast = true );
    void        SetError( SCROW nRow, const FormulaError nError);

    OUString    GetString( SCROW nRow, ScInterpreterContext* pContext = nullptr ) const
        { return GetString( GetCellValue( nRow ), nRow, pContext ); }
    OUString    GetString( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow,
                           ScInterpreterContext* pContext = nullptr ) const
        { return GetString( GetCellValue( rBlockPos, nRow ), nRow, pContext ); }
    double* GetValueCell( SCROW nRow );
    // Note that if pShared is set and a value is returned that way, the returned OUString is empty.
    OUString    GetInputString( SCROW nRow, bool bForceSystemLocale = false ) const
        { return GetInputString( GetCellValue( nRow ), nRow, bForceSystemLocale ); }
    OUString    GetInputString( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow,
                    bool bForceSystemLocale = false ) const
        { return GetInputString( GetCellValue( rBlockPos, nRow ), nRow, bForceSystemLocale ); }
    double      GetValue( SCROW nRow ) const;
    const EditTextObject* GetEditText( SCROW nRow ) const;
    void RemoveEditTextCharAttribs( SCROW nRow, const ScPatternAttr& rAttr );
    OUString GetFormula( SCROW nRow ) const;
    const ScFormulaCell* GetFormulaCell( SCROW nRow ) const;
    ScFormulaCell* GetFormulaCell( SCROW nRow );
    ScFormulaCell * const * GetFormulaCellBlockAddress( SCROW nRow, size_t& rBlockSize ) const;
    CellType    GetCellType( SCROW nRow ) const;
    SCSIZE      GetCellCount() const;
    sal_uInt64  GetWeightedCount() const;
    sal_uInt64  GetWeightedCount(SCROW nStartRow, SCROW nEndRow) const;
    sal_uInt64  GetCodeCount() const;       // RPN-Code in formulas
    FormulaError  GetErrCode( SCROW nRow ) const;

    bool    HasStringData( SCROW nRow ) const;
    bool    HasValueData( SCROW nRow ) const;
    bool    HasStringCells( SCROW nStartRow, SCROW nEndRow ) const;

    sc::MultiDataCellState::StateType HasDataCellsInRange(
        SCROW nRow1, SCROW nRow2, SCROW* pRow1 ) const;

    bool IsFormulaDirty( SCROW nRow ) const;

    void CheckVectorizationState();
    void SetAllFormulasDirty( const sc::SetFormulaDirtyContext& rCxt );
    void SetDirtyFromClip( SCROW nRow1, SCROW nRow2, sc::ColumnSpanSet& rBroadcastSpans );
    void SetDirty( SCROW nRow1, SCROW nRow2, BroadcastMode );
    void        SetDirtyVar();
    void        SetDirtyAfterLoad();
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
    bool BroadcastBroadcasters( SCROW nRow1, SCROW nRow2, SfxHintId nHint );

    bool CompileErrorCells( sc::CompileFormulaContext& rCxt, FormulaError nErrCode );

    void        ResetChanged( SCROW nStartRow, SCROW nEndRow );

    bool UpdateReferenceOnCopy( sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc = nullptr );

    /**
     * Update reference addresses in formula cell in response to mass cell
     * movement.
     *
     * @return true if reference of at least one formula cell has been
     *         updated, false otherwise.
     */
    bool UpdateReference( sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc );

    void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt );
    void UpdateInsertTabOnlyCells( sc::RefUpdateInsertTabContext& rCxt );
    void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt );
    void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt, SCTAB nTabNo );
    void        UpdateCompile( bool bForceIfNameInUse = false );
    void        UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc );
    void        UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void        SetTabNo(SCTAB nNewTab);
    void        FindRangeNamesInUse(SCROW nRow1, SCROW nRow2, sc::UpdatedRangeNames& rIndexes) const;

    void PreprocessRangeNameUpdate(
        sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt );

    void CompileHybridFormula(
        sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt );

    void PreprocessDBDataUpdate(
        sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt );

    using ScColumnData::GetAttr;
    using ScColumnData::GetPattern;
    using ScColumnData::GetNumberFormat;

    void        MergeSelectionPattern( ScMergePatternState& rState, const ScMarkData& rMark, bool bDeep ) const;
    void        MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                            ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight ) const;
    void        ApplyBlockFrame(const SvxBoxItem& rLineOuter, const SvxBoxInfoItem* pLineInner,
                                SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight);

    void        ApplyAttr( SCROW nRow, const SfxPoolItem& rAttr );
    void        ApplyPattern( SCROW nRow, const ScPatternAttr& rPatAttr );
    void        SetPattern( SCROW nRow, const CellAttributeHolder& rHolder );
    void        SetPattern( SCROW nRow, const ScPatternAttr& rPattern );
    void        SetPatternArea( SCROW nStartRow, SCROW nEndRow, const CellAttributeHolder& );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, SvNumFormatType nNewType );

    void        ApplyStyle( SCROW nRow, const ScStyleSheet* rStyle );
    void        ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const ::editeng::SvxBorderLine* pLine, bool bColorOnly );
    void        AddCondFormat(SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex );
    void        RemoveCondFormat(SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex );

    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark, bool& rFound ) const;
    const ScStyleSheet* GetAreaStyle( bool& rFound, SCROW nRow1, SCROW nRow2 ) const;

    void        FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset );
    bool        IsStyleSheetUsed( const ScStyleSheet& rStyle ) const;

                /// May return -1 if not found
    SCROW SearchStyle(
        SCROW nRow, const ScStyleSheet* pSearchStyle, bool bUp, bool bInSelection,
        const ScMarkData& rMark) const;

    bool SearchStyleRange(
        SCROW& rRow, SCROW& rEndRow, const ScStyleSheet* pSearchStyle, bool bUp,
        bool bInSelection, const ScMarkData& rMark) const;

    bool        ApplyFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags );
    bool        RemoveFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags );
    void        ClearItems( SCROW nStartRow, SCROW nEndRow, const sal_uInt16* pWhich );

    void        RemoveProtected( SCROW nStartRow, SCROW nEndRow );

    void DeleteSelection( InsertDeleteFlags nDelFlag, const ScMarkData& rMark, bool bBroadcast );

    tools::Long GetNeededSize(
        SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY,
        const Fraction& rZoomX, const Fraction& rZoomY,
        bool bWidth, const ScNeededSizeOptions& rOptions, const ScPatternAttr** pPatternChange,
        bool bInPrintTwips = false ) const;

    sal_uInt16 GetOptimalColWidth(
        OutputDevice* pDev, double nPPTX, double nPPTY,
        const Fraction& rZoomX, const Fraction& rZoomY,
        bool bFormula, sal_uInt16 nOldWidth, const ScMarkData* pMarkData, const ScColWidthParam* pParam) const;

    void GetOptimalHeight(
        sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow, sal_uInt16 nMinHeight, SCROW nMinStart );

    void GetFilterEntries(
        sc::ColumnBlockConstPosition& rBlockPos, SCROW nStartRow, SCROW nEndRow,
        ScFilterEntries& rFilterEntries, bool bFiltering, bool bFilteredRow );

    void GetBackColorFilterEntries(SCROW nRow1, SCROW nRow2, ScFilterEntries& rFilterEntries );

    bool GetDataEntries( SCROW nRow, std::set<ScTypedStrData>& rStrings) const;

    void UpdateInsertTabAbs(SCTAB nNewPos);
    bool    TestTabRefAbs(SCTAB nTable) const;
    bool    GetNextSpellingCell(SCROW& nRow, bool bInSel, const ScMarkData& rData) const;

    void StartListeningFormulaCells(
        sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt, SCROW nRow1, SCROW nRow2 );

    void EndListeningFormulaCells(
        sc::EndListeningContext& rCxt, SCROW nRow1, SCROW nRow2,
        SCROW* pStartRow, SCROW* pEndRow );

    void        StartListening( SvtListener& rLst, SCROW nRow );
    void        EndListening( SvtListener& rLst, SCROW nRow );
    void StartListening( sc::StartListeningContext& rCxt, const ScAddress& rAddress, SvtListener& rListener );
    void EndListening( sc::EndListeningContext& rCxt, const ScAddress& rAddress, SvtListener& rListener );
    void StartListeners( sc::StartListeningContext& rCxt, bool bAll );
    void        SetDirtyIfPostponed();
    void BroadcastRecalcOnRefMove();
    void CollectListeners( std::vector<SvtListener*>& rListeners, SCROW nRow1, SCROW nRow2 );
    void CollectFormulaCells( std::vector<ScFormulaCell*>& rCells, SCROW nRow1, SCROW nRow2 );

    void CompileDBFormula( sc::CompileFormulaContext& rCxt );
    void CompileColRowNameFormula( sc::CompileFormulaContext& rCxt );

    sal_Int32   GetMaxStringLen( SCROW nRowStart, SCROW nRowEnd, rtl_TextEncoding eCharSet ) const;
    sal_Int32   GetMaxNumberStringLen( sal_uInt16& nPrecision,
                                       SCROW nRowStart, SCROW nRowEnd ) const;

    sal_uInt16 GetTextWidth(SCROW nRow) const;
    void SetTextWidth(SCROW nRow, sal_uInt16 nWidth);

    SvtScriptType GetScriptType( SCROW nRow ) const;

    /**
     * Get combined script types of the specified range. This method may
     * update script types on demand if they have not been determined.
     */
    SvtScriptType GetRangeScriptType( sc::CellTextAttrStoreType::iterator& itPos, SCROW nRow1, SCROW nRow2,
            const sc::CellStoreType::iterator& itr);

    void SetScriptType( SCROW nRow, SvtScriptType nType );
    void UpdateScriptTypes( SCROW nRow1, SCROW nRow2 );

    formula::FormulaTokenRef ResolveStaticReference( SCROW nRow );
    bool ResolveStaticReference( ScMatrix& rMat, SCCOL nMatCol, SCROW nRow1, SCROW nRow2 );
    void FillMatrix( ScMatrix& rMat, size_t nMatCol, SCROW nRow1, SCROW nRow2, svl::SharedStringPool* pPool ) const;
    formula::VectorRefArray FetchVectorRefArray( SCROW nRow1, SCROW nRow2 );
    bool HandleRefArrayForParallelism( SCROW nRow1, SCROW nRow2,
                                       const ScFormulaCellGroupRef& mxGroup, ScAddress* pDirtiedAddress );
#ifdef DBG_UTIL
    void AssertNoInterpretNeeded( SCROW nRow1, SCROW nRow2 );
#endif
    void SetFormulaResults( SCROW nRow, const double* pResults, size_t nLen );

    void CalculateInThread( ScInterpreterContext& rContext, SCROW nRow, size_t nLen, size_t nOffset,
                            unsigned nThisThread, unsigned nThreadsTotal );
    void HandleStuffAfterParallelCalculation( SCROW nRow, size_t nLen, ScInterpreter* pInterpreter );

    void SetNumberFormat( SCROW nRow, sal_uInt32 nNumberFormat );

    SvtBroadcaster* GetBroadcaster( SCROW nRow );
    const SvtBroadcaster* GetBroadcaster( SCROW nRow ) const;

    void DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2 );
    void PrepareBroadcastersForDestruction();
    void DeleteEmptyBroadcasters();

    void Broadcast( SCROW nRow );
    void BroadcastCells( const std::vector<SCROW>& rRows, SfxHintId nHint );
    void BroadcastRows( SCROW nStartRow, SCROW nEndRow, SfxHintId nHint );

    // Spaklines
    sc::SparklineCell* GetSparklineCell(SCROW nRow);
    void CreateSparklineCell(SCROW nRow, std::shared_ptr<sc::Sparkline> const& pSparkline);
    void DeleteSparklineCells(sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2);
    bool DeleteSparkline(SCROW nRow);
    bool IsSparklinesEmptyBlock(SCROW nStartRow, SCROW nEndRow) const;
    void CopyCellSparklinesToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol, SCROW nRowOffsetDest = 0) const;
    void DuplicateSparklines(SCROW nStartRow, size_t nDataSize, ScColumn& rDestCol,
                             sc::ColumnBlockPosition& rDestBlockPos, SCROW nRowOffsetDest = 0) const;
    bool HasSparklines() const;
    SCROW GetSparklinesMaxRow() const;
    SCROW GetSparklinesMinRow() const;

    // cell notes
    ScPostIt* GetCellNote( SCROW nRow );
    const ScPostIt* GetCellNote( SCROW nRow ) const;
    ScPostIt* GetCellNote( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow );
    const ScPostIt* GetCellNote( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow ) const;
    void DeleteCellNotes( sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2, bool bForgetCaptionOwnership );
    bool HasCellNote(SCROW nStartRow, SCROW nEndRow) const;
    bool HasCellNotes() const;
    void SetCellNote( SCROW nRow, std::unique_ptr<ScPostIt> pNote);
    bool IsNotesEmptyBlock(SCROW nStartRow, SCROW nEndRow) const;

    std::unique_ptr<ScPostIt> ReleaseNote( SCROW nRow );
    size_t GetNoteCount() const;
    void CreateAllNoteCaptions();
    void ForgetNoteCaptions( SCROW nRow1, SCROW nRow2, bool bPreserveData );
    SCROW GetNotePosition( size_t nIndex ) const;
    void GetAllNoteEntries( std::vector<sc::NoteEntry>& rNotes ) const;
    void GetNotesInRange( SCROW nStartRow, SCROW nEndRow, std::vector<sc::NoteEntry>& rNotes ) const;

    SCROW GetCellNotesMaxRow() const;
    SCROW GetCellNotesMinRow() const;

    void CopyCellNotesToDocument(
        SCROW nRow1, SCROW nRow2, ScColumn& rDestCol, bool bCloneCaption = true,
        SCROW nRowOffsetDest = 0) const;

    void DuplicateNotes(SCROW nStartRow, size_t nDataSize, ScColumn& rDestCol,
                            sc::ColumnBlockPosition& rDestBlockPos, bool bCloneCaption, SCROW nRowOffsetDest = 0) const;

    void UpdateNoteCaptions( SCROW nRow1, SCROW nRow2, bool bAddressChanged = true );
    void CommentNotifyAddressChange( SCROW nRow1, SCROW nRow2 );

    void UpdateDrawObjects( std::vector<std::vector<SdrObject*>>& pObjects, SCROW nRowStart, SCROW nRowEnd );
    void UpdateDrawObjectsForRow( std::vector<SdrObject*>& pObjects, SCCOL nTargetCol, SCROW nTargetRow );
    bool IsDrawObjectsEmptyBlock(SCROW nStartRow, SCROW nEndRow) const;

    void InterpretDirtyCells( SCROW nRow1, SCROW nRow2 );
    bool InterpretCellsIfNeeded( SCROW nRow1, SCROW nRow2 );

    static void JoinNewFormulaCell( const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell );

    /**
     * Detach a formula cell that's about to be deleted, or removed from
     * document storage (if that ever happens).
     *
     * @param rNewSharedRows collects possible new shared row ranges (top and
     *        bottom of shared or remaining single twice) resulting from
     *        unsharing to reestablish listeners on.
     */
    void DetachFormulaCell( const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell,
                            std::vector<SCROW>& rNewSharedRows );

    /** Re-establish listeners on unshared formula groups */
    void StartListeningUnshared( const std::vector<SCROW>& rNewSharedRows );

    void DetachFormulaCells( const sc::CellStoreType::position_type& aPos, size_t nLength,
                             std::vector<SCROW>* pNewSharedRows );

    void AttachFormulaCells( sc::StartListeningContext& rCxt, SCROW nRow1, SCROW nRow2 );
    void DetachFormulaCells( sc::EndListeningContext& rCxt, SCROW nRow1, SCROW nRow2 );

    /**
     * Regroup formula cells for the entire column.
     */
    void RegroupFormulaCells( std::vector<ScAddress>* pGroupPos = nullptr );

    /**
     * Reset column position of formula cells within specified row range.
     * If bUpdateRefs==true then reference positions are also adjusted to
     * reflect the new position so that the formula cells still reference the
     * same cells or ranges after the position change.
     * The position of a formula cell before the call is interpreted as the old
     * position of that cell.
     *
     * Caller needs to ensure that no formula groups cross the top and bottom
     * row boundaries.
     *
     * @param nRow1 top row boundary
     * @param nRow2 bottom row boundary
     * @param bUpdateRefs whether to adjust references
     */
    void ResetFormulaCellPositions( SCROW nRow1, SCROW nRow2, bool bUpdateRefs );

    void SplitFormulaGroupByRelativeRef( const ScRange& rBoundRange );

    void TransferCellValuesTo( SCROW nRow, size_t nLen, sc::CellValues& rDest );
    void CopyCellValuesFrom( SCROW nRow, const sc::CellValues& rSrc );
    void ConvertFormulaToValue(
        sc::EndListeningContext& rCxt, SCROW nRow1, SCROW nRow2, sc::TableValues* pUndo );

    void SwapNonEmpty(
        sc::TableValues& rValues, sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt );

    std::optional<sc::ColumnIterator> GetColumnIterator( SCROW nRow1, SCROW nRow2 ) const;

    bool EnsureFormulaCellResults( SCROW nRow1, SCROW nRow2, bool bSkipRunning = false );

    void StoreToCache(SvStream& rStrm) const;
    void RestoreFromCache(SvStream& rStrm);

#if DUMP_COLUMN_STORAGE
    void DumpColumnStorage() const;
#endif

    SCSIZE      GetPatternCount() const;
    SCSIZE      GetPatternCount( SCROW nRow1, SCROW nRow2 ) const;
    bool        ReservePatternCount( SCSIZE nReserve );

    void CheckIntegrity() const;

private:

    sc::CellStoreType::iterator GetPositionToInsert( SCROW nRow, std::vector<SCROW>& rNewSharedRows,
                                                     bool bInsertFormula );
    sc::CellStoreType::iterator GetPositionToInsert( const sc::CellStoreType::iterator& it, SCROW nRow,
                                                     std::vector<SCROW>& rNewSharedRows, bool bInsertFormula );

    void AttachNewFormulaCell(
        const sc::CellStoreType::iterator& itPos, SCROW nRow, ScFormulaCell& rCell,
        const std::vector<SCROW>& rNewSharedRows,
        bool bJoin = true, sc::StartListeningType eListenType = sc::SingleCellListening );

    void AttachNewFormulaCell(
        const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell,
        const std::vector<SCROW>& rNewSharedRows,
        bool bJoin = true, sc::StartListeningType eListenType = sc::SingleCellListening );

public:
    void AttachNewFormulaCells(const sc::CellStoreType::position_type& aPos, size_t nLength,
                               std::vector<SCROW>& rNewSharedRows);

private:
    void BroadcastNewCell( SCROW nRow );
    bool UpdateScriptType( sc::CellTextAttr& rAttr, SCROW nRow, sc::CellStoreType::iterator& itr );

    const ScFormulaCell* FetchFormulaCell( SCROW nRow ) const;

    SCROW FindNextVisibleRowWithContent(
        sc::CellStoreType::const_iterator& itPos, SCROW nRow, bool bForward) const;
    SCROW FindNextVisibleRow(SCROW nRow, bool bForward) const;

    OUString GetString( const ScRefCellValue& cell, SCROW nRow, ScInterpreterContext* pContext = nullptr ) const;
    OUString GetInputString( const ScRefCellValue& cell, SCROW nRow, bool bForceSystemLocale = false ) const;

    /**
     * Called whenever the state of cell array gets modified i.e. new cell
     * insertion, cell removal or relocation, cell value update and so on.
     *
     * Call this only from those methods where maCells is modified directly.
     */
    void CellStorageModified();
    void CellNotesDeleting(SCROW nRow1, SCROW nRow2, bool bForgetCaptionOwnership );

    void CopyCellTextAttrsToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol) const;

    struct DeleteCellsResult
    {
        /** cell ranges that have been deleted. */
        sc::SingleColumnSpanSet aDeletedRows;
        /** formula cell range that has stopped listening. */
        std::vector<std::pair<SCROW, SCROW>> aFormulaRanges;

        DeleteCellsResult( const ScDocument& rDoc );
        DeleteCellsResult( const DeleteCellsResult& ) = delete;
    };

    std::unique_ptr<DeleteCellsResult> DeleteCells(
        sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2, InsertDeleteFlags nDelFlag );

    /**
     * Get all non-grouped formula cells and formula cell groups in the whole
     * column.
     */
    std::vector<sc::FormulaGroupEntry> GetFormulaGroupEntries();

    void EndListeningIntersectedGroup(
        sc::EndListeningContext& rCxt, SCROW nRow, std::vector<ScAddress>* pGroupPos );

    void EndListeningIntersectedGroups(
        sc::EndListeningContext& rCxt, SCROW nRow1, SCROW nRow2, std::vector<ScAddress>* pGroupPos );

    void EndListeningGroup( sc::EndListeningContext& rCxt, SCROW nRow );
    void SetNeedsListeningGroup( SCROW nRow );

    void CollectBroadcasterState(sc::BroadcasterState& rState) const;
};

inline bool ScColumn::IsEmptyAttr() const
{
    return pAttrArray->IsEmpty();
}

inline bool ScColumnData::IsAllAttrEqual( const ScColumnData& rCol, SCROW nStartRow, SCROW nEndRow ) const
{
    return pAttrArray->IsAllEqual( *rCol.pAttrArray, nStartRow, nEndRow );
}

inline bool ScColumn::IsVisibleAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const
{
    return pAttrArray->IsVisibleEqual( *rCol.pAttrArray, nStartRow, nEndRow );
}

inline bool ScColumn::GetFirstVisibleAttr( SCROW& rFirstRow ) const
{
    return pAttrArray->GetFirstVisibleAttr( rFirstRow );
}

inline bool ScColumn::GetLastVisibleAttr( SCROW& rLastRow, bool bSkipEmpty ) const
{
    // row of last cell is needed
    SCROW nLastData = GetLastDataPos();    // always including notes, 0 if none
    return pAttrArray->GetLastVisibleAttr( rLastRow, nLastData, bSkipEmpty );
}

inline bool ScColumn::HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const
{
    return pAttrArray->HasVisibleAttrIn( nStartRow, nEndRow );
}

inline SCSIZE ScColumn::GetPatternCount() const
{
    return pAttrArray->Count();
}

inline SCSIZE ScColumn::GetPatternCount( SCROW nRow1, SCROW nRow2 ) const
{
    return pAttrArray->Count( nRow1, nRow2 );
}

inline bool ScColumn::ReservePatternCount( SCSIZE nReserve )
{
    return pAttrArray && pAttrArray->Reserve( nReserve );
}

inline bool ScColumn::IsMerged( SCROW nRow ) const
{
    return pAttrArray->IsMerged(nRow);
}

inline SCROW ScColumnData::GetNextUnprotected( SCROW nRow, bool bUp ) const
{
    return pAttrArray->GetNextUnprotected(nRow, bUp);
}

inline bool ScColumnData::HasAttrib( SCROW nRow1, SCROW nRow2, HasAttrFlags nMask ) const
{
    return pAttrArray->HasAttrib( nRow1, nRow2, nMask );
}

inline bool ScColumnData::HasAttrib( SCROW nRow, HasAttrFlags nMask, SCROW* nStartRow, SCROW* nEndRow ) const
{
    return pAttrArray->HasAttrib( nRow, nMask, nStartRow, nEndRow );
}

inline bool ScColumn::ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                            SCCOL& rPaintCol, SCROW& rPaintRow,
                            bool bRefresh )
{
    return pAttrArray->ExtendMerge( nThisCol, nStartRow, nEndRow, rPaintCol, rPaintRow, bRefresh );
}

inline void ScColumnData::MergePatternArea( ScMergePatternState& rState, SCROW nRow1, SCROW nRow2, bool bDeep ) const
{
    pAttrArray->MergePatternArea( nRow1, nRow2, rState, bDeep );
}

inline void ScColumn::MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                            ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight ) const
{
    pAttrArray->MergeBlockFrame( pLineOuter, pLineInner, rFlags, nStartRow, nEndRow, bLeft, nDistRight );
}

inline void ScColumn::ApplyBlockFrame(const SvxBoxItem& rLineOuter, const SvxBoxInfoItem* pLineInner,
                               SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight)
{
    pAttrArray->ApplyBlockFrame(rLineOuter, pLineInner, nStartRow, nEndRow, bLeft, nDistRight);
}

inline const ScPatternAttr* ScColumnData::GetPattern( SCROW nRow ) const
{
    return pAttrArray->GetPattern( nRow );
}

inline const SfxPoolItem& ScColumnData::GetAttr( SCROW nRow, sal_uInt16 nWhich ) const
{
    return pAttrArray->GetPattern( nRow )->GetItem(nWhich);
}

inline const SfxPoolItem& ScColumnData::GetAttr( SCROW nRow, sal_uInt16 nWhich, SCROW& nStartRow, SCROW& nEndRow ) const
{
    return pAttrArray->GetPatternRange( nStartRow, nEndRow, nRow )->GetItem(nWhich);
}

inline sal_uInt32 ScColumnData::GetNumberFormat( const ScInterpreterContext& rContext, SCROW nRow ) const
{
    return pAttrArray->GetPattern( nRow )->GetNumberFormat( rContext );
}

inline void ScColumn::AddCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    pAttrArray->AddCondFormat( nStartRow, nEndRow, nIndex );
}

inline void ScColumn::RemoveCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    pAttrArray->RemoveCondFormat( nStartRow, nEndRow, nIndex );
}

inline void ScColumnData::ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, const ScStyleSheet& rStyle )
{
    pAttrArray->ApplyStyleArea(nStartRow, nEndRow, rStyle);
}

inline const ScStyleSheet* ScColumnData::GetStyle( SCROW nRow ) const
{
    return pAttrArray->GetPattern( nRow )->GetStyleSheet();
}

inline void ScColumn::FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset )
{
    pAttrArray->FindStyleSheet( pStyleSheet, rUsedRows, bReset );
}

inline bool ScColumn::IsStyleSheetUsed( const ScStyleSheet& rStyle ) const
{
    return pAttrArray->IsStyleSheetUsed( rStyle );
}

inline bool ScColumn::ApplyFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags )
{
    return pAttrArray->ApplyFlags( nStartRow, nEndRow, nFlags );
}

inline bool ScColumn::RemoveFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags )
{
    return pAttrArray->RemoveFlags( nStartRow, nEndRow, nFlags );
}

inline void ScColumn::ClearItems( SCROW nStartRow, SCROW nEndRow, const sal_uInt16* pWhich )
{
    pAttrArray->ClearItems( nStartRow, nEndRow, pWhich );
}

inline void ScColumn::SetPattern( SCROW nRow, const CellAttributeHolder& rHolder )
{
    return pAttrArray->SetPattern( nRow, rHolder );
}

inline void ScColumn::SetPattern( SCROW nRow, const ScPatternAttr& rPattern )
{
    pAttrArray->SetPattern( nRow, CellAttributeHolder(&rPattern) );
}

inline void ScColumn::SetPatternArea( SCROW nStartRow, SCROW nEndRow, const CellAttributeHolder& rHolder )
{
    pAttrArray->SetPatternArea( nStartRow, nEndRow, rHolder );
}

inline void ScColumnData::SetAttrEntries(std::vector<ScAttrEntry> && vNewData)
{
    pAttrArray->SetAttrEntries( std::move( vNewData ));
}

inline bool ScColumnData::TestInsertRow( SCSIZE nSize ) const
{
    return pAttrArray->TestInsertRow( nSize );
}

inline void ScColumnData::InsertRow( SCROW nStartRow, SCSIZE nSize )
{
    pAttrArray->InsertRow( nStartRow, nSize );
}

inline void ScColumnData::DeleteRow(SCROW nStartRow, SCSIZE nSize)
{
    pAttrArray->DeleteRow( nStartRow, nSize );
}

template <typename ApplyDataFunc>
void ScColumnData::Apply(const ScMarkData& rMark, SCCOL nCol, ApplyDataFunc apply)
{
    if (rMark.IsMultiMarked())
    {
        ScMultiSelIter aMultiIter(rMark.GetMultiSelData(), nCol);
        SCROW nTop, nBottom;
        while (aMultiIter.Next(nTop, nBottom))
            apply(*this, nTop, nBottom);
    }
    else if (rMark.IsMarked())
    {
        const ScRange& aRange = rMark.GetMarkArea();
        if (aRange.aStart.Col() <= nCol && nCol <= aRange.aEnd.Col())
            apply(*this, aRange.aStart.Row(), aRange.aEnd.Row());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
