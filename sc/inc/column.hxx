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

#ifndef INCLUDED_SC_INC_COLUMN_HXX
#define INCLUDED_SC_INC_COLUMN_HXX

#include "global.hxx"
#include "address.hxx"
#include "rangelst.hxx"
#include "types.hxx"
#include "mtvelements.hxx"
#include <formula/types.hxx>
#include <svl/zforlist.hxx>
#include <svx/svdobj.hxx>
#include "attarray.hxx"

#include <set>
#include <vector>

#include <mdds/flat_segment_tree.hpp>

namespace editeng { class SvxBorderLine; }
namespace formula { struct VectorRefArray; }

namespace sc {

struct FormulaGroupEntry;
class StartListeningContext;
class EndListeningContext;
class CopyFromClipContext;
class CopyToClipContext;
class CopyToDocContext;
class MixDocContext;
class ColumnSpanSet;
class SingleColumnSpanSet;
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

}

class Fraction;
class OutputDevice;
class SfxItemPoolCache;
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
    const ScPatternAttr*    pPattern;
    bool                bFormula;
    bool                bSkipMerged;
    bool                bGetFont;
    bool                bTotalSize;

    ScNeededSizeOptions();
};

class ScColumn
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

    sc::CellStoreEvent const maCellsEvent;

    // Cell values.
    sc::CellStoreType maCells;

    std::unique_ptr<ScAttrArray> pAttrArray;

    size_t mnBlkCountFormula;

    SCCOL           nCol;
    SCTAB           nTab;

friend class ScDocument;                    // for FillInfo
friend class ScTable;
friend class ScValueIterator;
friend class ScHorizontalValueIterator;
friend class ScDBQueryDataIterator;
friend class ScQueryCellIterator;
friend class ScFormulaGroupIterator;
friend class ScCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScColumnTextWidthIterator;
friend class ScDocumentImport;
friend class sc::DocumentStreamAccess;
friend class sc::SingleColumnSpanSet;
friend class sc::ColumnSpanSet;
friend class sc::EditTextIterator;
friend class sc::CellValues;
friend class sc::TableValues;
friend class sc::CellStoreEvent;

    ScColumn(const ScColumn&) = delete;
    ScColumn& operator= (const ScColumn&) = delete;

    bool ParseString(
        ScCellValue& rCell,
        SCROW nRow, SCTAB nTab, const OUString& rString, formula::FormulaGrammar::AddressConvention eConv,
        const ScSetStringParam* pParam );

public:

    /** Broadcast mode for SetDirty(SCROW,SCROW,BroadcastMode). */
    enum BroadcastMode
    {
        BROADCAST_NONE,             ///< no broadcasting
        BROADCAST_DATA_POSITIONS,   ///< broadcast existing cells with position => does AreaBroadcast
        BROADCAST_BROADCASTERS      ///< broadcast only existing cell broadcasters => no AreaBroadcast of range!
    };

    ScColumn();
    ~ScColumn() COVERITY_NOEXCEPT_FALSE;

    void        Init(SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc, bool bEmptyAttrArray);

    ScDocument* GetDoc() const { return pAttrArray->GetDoc(); }
    SCTAB GetTab() const { return nTab; }
    SCCOL GetCol() const { return nCol; }
    sc::CellStoreType& GetCellStore() { return maCells; }
    const sc::CellStoreType& GetCellStore() const { return maCells; }
    sc::CellTextAttrStoreType& GetCellAttrStore() { return maCellTextAttrs; }
    const sc::CellTextAttrStoreType& GetCellAttrStore() const { return maCellTextAttrs; }
    sc::CellNoteStoreType& GetCellNoteStore() { return maCellNotes; }
    const sc::CellNoteStoreType& GetCellNoteStore() const { return maCellNotes; }

    ScRefCellValue GetCellValue( SCROW nRow ) const;
    ScRefCellValue GetCellValue( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow ) const;
    static ScRefCellValue GetCellValue( const sc::CellStoreType::const_iterator& itPos, size_t nOffset );

    const sc::CellTextAttr* GetCellTextAttr( SCROW nRow ) const;
    const sc::CellTextAttr* GetCellTextAttr( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow ) const;

    void        Delete( SCROW nRow );
    void        DeleteContent( SCROW nRow, bool bBroadcast = true );
    void        FreeAll();
    void        FreeNotes();
    void        Swap( ScColumn& rOther, SCROW nRow1, SCROW nRow2, bool bPattern );

    bool        HasAttrib( SCROW nRow1, SCROW nRow2, HasAttrFlags nMask ) const;
    bool        HasAttribSelection( const ScMarkData& rMark, HasAttrFlags nMask ) const;
    bool        IsMerged( SCROW nRow ) const;
    bool        ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                bool bRefresh );

    bool        IsEmptyData() const;
    bool        IsEmptyAttr() const;

                // data only:
    bool        IsEmptyBlock(SCROW nStartRow, SCROW nEndRow) const;
    SCSIZE      GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const;
    bool        HasDataAt(SCROW nRow, bool bConsiderCellNotes=false,
                          bool bConsiderCellDrawObjects=false) const;
    bool        HasVisibleDataAt(SCROW nRow) const;
    SCROW       GetFirstDataPos() const;
    SCROW       GetLastDataPos() const;
    SCROW       GetLastDataPos( SCROW nLastRow, bool bConsiderCellNotes=false,
                                bool bConsiderCellDrawObjects=false ) const;
    bool        GetPrevDataPos(SCROW& rRow) const;
    bool        GetNextDataPos(SCROW& rRow) const;
    bool        TrimEmptyBlocks(SCROW& rRowStart, SCROW& rRowEnd) const;
    void        FindDataAreaPos(SCROW& rRow, bool bDown) const; // (without Broadcaster)
    void        FindUsed( SCROW nStartRow, SCROW nEndRow, mdds::flat_segment_tree<SCROW, bool>& rUsed ) const;

    SCSIZE             VisibleCount( SCROW nStartRow, SCROW nEndRow ) const;
    sc::MatrixEdge     GetBlockMatrixEdges(SCROW nRow1, SCROW nRow2, sc::MatrixEdge nMask ) const;
    bool    HasSelectionMatrixFragment(const ScMarkData& rMark) const;

    bool    GetFirstVisibleAttr( SCROW& rFirstRow ) const;
    bool    GetLastVisibleAttr( SCROW& rLastRow ) const;
    bool    HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const;
    bool    IsVisibleAttrEqual( const ScColumn& rCol, SCROW nStartRow = 0,
                                    SCROW nEndRow = MAXROW ) const;
    bool    IsAllAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const;

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
        sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2, long nDy, ScColumn& rColumn );

    void        RemoveEditAttribs( SCROW nStartRow, SCROW nEndRow );

                //  Selection (?) of this document
    void MixMarked(
        sc::MixDocContext& rCxt, const ScMarkData& rMark, ScPasteFunc nFunction,
        bool bSkipEmpty, const ScColumn& rSrcCol );
    void MixData(
        sc::MixDocContext& rCxt, SCROW nRow1, SCROW nRow2, ScPasteFunc nFunction, bool bSkipEmpty,
        const ScColumn& rSrcCol );

    ScAttrIterator* CreateAttrIterator( SCROW nStartRow, SCROW nEndRow ) const;

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

    void        CopyUpdated( const ScColumn& rPosCol, ScColumn& rDestCol ) const;

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

    void        GetString( SCROW nRow, OUString& rString, const ScInterpreterContext* pContext = nullptr ) const;
    double* GetValueCell( SCROW nRow );
    void        GetInputString( SCROW nRow, OUString& rString ) const;
    double      GetValue( SCROW nRow ) const;
    const EditTextObject* GetEditText( SCROW nRow ) const;
    void RemoveEditTextCharAttribs( SCROW nRow, const ScPatternAttr& rAttr );
    void        GetFormula( SCROW nRow, OUString& rFormula ) const;
    const ScFormulaCell* GetFormulaCell( SCROW nRow ) const;
    ScFormulaCell* GetFormulaCell( SCROW nRow );
    ScFormulaCell * const * GetFormulaCellBlockAddress( SCROW nRow, size_t& rBlockSize ) const;
    CellType    GetCellType( SCROW nRow ) const;
    SCSIZE      GetCellCount() const;
    sal_uLong GetWeightedCount() const;
    sal_uLong GetWeightedCount(SCROW nStartRow, SCROW nEndRow) const;
    sal_uInt32 GetCodeCount() const;       // RPN-Code in formulas
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
    bool BroadcastBroadcasters( SCROW nRow1, SCROW nRow2, ScHint& rHint );

    bool CompileErrorCells( sc::CompileFormulaContext& rCxt, FormulaError nErrCode );

    void        ResetChanged( SCROW nStartRow, SCROW nEndRow );

    bool UpdateReferenceOnCopy( const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc = nullptr );

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

    const SfxPoolItem&      GetAttr( SCROW nRow, sal_uInt16 nWhich ) const;
    template<class T> const T&  GetAttr( SCROW nRow, TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T&>(GetAttr(nRow, sal_uInt16(nWhich)));
    }
    const ScPatternAttr*    GetPattern( SCROW nRow ) const;
    const ScPatternAttr*    GetMostUsedPattern( SCROW nStartRow, SCROW nEndRow ) const;

    sal_uInt32 GetNumberFormat( const ScInterpreterContext& rContext, SCROW nRow ) const;
    sal_uInt32  GetNumberFormat( SCROW nStartRow, SCROW nEndRow ) const;

    void        MergeSelectionPattern( ScMergePatternState& rState, const ScMarkData& rMark, bool bDeep ) const;
    void        MergePatternArea( ScMergePatternState& rState, SCROW nRow1, SCROW nRow2, bool bDeep ) const;
    void        MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                            ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight ) const;
    void        ApplyBlockFrame(const SvxBoxItem& rLineOuter, const SvxBoxInfoItem* pLineInner,
                                SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight);

    void        ApplyAttr( SCROW nRow, const SfxPoolItem& rAttr );
    void        ApplyPattern( SCROW nRow, const ScPatternAttr& rPatAttr );
    void        ApplyPatternArea( SCROW nStartRow, SCROW nEndRow, const ScPatternAttr& rPatAttr,
                                  ScEditDataArray* pDataArray = nullptr,
                                  bool* const pIsChanged = nullptr);
    void        SetPattern( SCROW nRow, const ScPatternAttr& rPatAttr );
    void        SetPatternArea( SCROW nStartRow, SCROW nEndRow,
                                const ScPatternAttr& rPatAttr );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, SvNumFormatType nNewType );

    void        ApplyStyle( SCROW nRow, const ScStyleSheet* rStyle );
    void        ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, const ScStyleSheet& rStyle );
    void        ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);
    void        ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const ::editeng::SvxBorderLine* pLine, bool bColorOnly );
    void        AddCondFormat(SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex );
    void        RemoveCondFormat(SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex );

    const ScStyleSheet* GetStyle( SCROW nRow ) const;
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

    SCROW       ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark, ScEditDataArray* pDataArray, bool* const pIsChanged );
    void DeleteSelection( InsertDeleteFlags nDelFlag, const ScMarkData& rMark, bool bBroadcast );

    void        ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark );
    void        ChangeSelectionIndent( bool bIncrement, const ScMarkData& rMark );

    long GetNeededSize(
        SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY,
        const Fraction& rZoomX, const Fraction& rZoomY,
        bool bWidth, const ScNeededSizeOptions& rOptions, const ScPatternAttr** pPatternChange ) const;

    sal_uInt16 GetOptimalColWidth(
        OutputDevice* pDev, double nPPTX, double nPPTY,
        const Fraction& rZoomX, const Fraction& rZoomY,
        bool bFormula, sal_uInt16 nOldWidth, const ScMarkData* pMarkData, const ScColWidthParam* pParam) const;

    void GetOptimalHeight(
        sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow, sal_uInt16 nMinHeight, SCROW nMinStart );

                /// Including current, may return -1
    SCROW      GetNextUnprotected( SCROW nRow, bool bUp ) const;

    void GetFilterEntries(
        sc::ColumnBlockConstPosition& rBlockPos, SCROW nStartRow, SCROW nEndRow,
        ScFilterEntries& rFilterEntries );

    bool GetDataEntries( SCROW nRow, std::set<ScTypedStrData>& rStrings, bool bLimit ) const;

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
    void TransferListeners( ScColumn& rDestCol, SCROW nRow1, SCROW nRow2, SCROW nRowDelta );
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

    size_t GetFormulaHash( SCROW nRow ) const;

    ScFormulaVectorState GetFormulaVectorState( SCROW nRow ) const;
    formula::FormulaTokenRef ResolveStaticReference( SCROW nRow );
    bool ResolveStaticReference( ScMatrix& rMat, SCCOL nMatCol, SCROW nRow1, SCROW nRow2 );
    void FillMatrix( ScMatrix& rMat, size_t nMatCol, SCROW nRow1, SCROW nRow2, svl::SharedStringPool* pPool ) const;
    formula::VectorRefArray FetchVectorRefArray( SCROW nRow1, SCROW nRow2 );
    bool HandleRefArrayForParallelism( SCROW nRow1, SCROW nRow2, const ScFormulaCellGroupRef& mxGroup );
#ifdef DBG_UTIL
    void AssertNoInterpretNeeded( SCROW nRow1, SCROW nRow2 );
#endif
    void SetFormulaResults( SCROW nRow, const double* pResults, size_t nLen );

    void CalculateInThread( ScInterpreterContext& rContext, SCROW nRow, size_t nLen, unsigned nThisThread, unsigned nThreadsTotal );
    void HandleStuffAfterParallelCalculation( SCROW nRow, size_t nLen );

    void SetNumberFormat( SCROW nRow, sal_uInt32 nNumberFormat );

    SvtBroadcaster* GetBroadcaster( SCROW nRow );
    const SvtBroadcaster* GetBroadcaster( SCROW nRow ) const;

    void DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2 );
    void PrepareBroadcastersForDestruction();

    void Broadcast( SCROW nRow );
    void BroadcastCells( const std::vector<SCROW>& rRows, SfxHintId nHint );
    void BroadcastRows( SCROW nStartRow, SCROW nEndRow, SfxHintId nHint );

    // cell notes
    ScPostIt* GetCellNote( SCROW nRow );
    const ScPostIt* GetCellNote( SCROW nRow ) const;
    const ScPostIt* GetCellNote( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow ) const;
    void DeleteCellNotes( sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2, bool bForgetCaptionOwnership );
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
                            sc::ColumnBlockPosition& maDestBlockPos, bool bCloneCaption, SCROW nRowOffsetDest=0 ) const;

    void UpdateNoteCaptions( SCROW nRow1, SCROW nRow2 );

    void UpdateDrawObjects( std::vector<std::vector<SdrObject*>>& pObjects, SCROW nRowStart, SCROW nRowEnd );
    void UpdateDrawObjectsForRow( std::vector<SdrObject*>& pObjects, SCCOL nTargetCol, SCROW nTargetRow );
    bool IsDrawObjectsEmptyBlock(SCROW nStartRow, SCROW nEndRow) const;

    void InterpretDirtyCells( SCROW nRow1, SCROW nRow2 );

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
    void DetachFormulaCells( sc::EndListeningContext& rCxt, SCROW nRow1, SCROW nRow2,
                             std::vector<SCROW>* pNewSharedRows );

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

    std::unique_ptr<sc::ColumnIterator> GetColumnIterator( SCROW nRow1, SCROW nRow2 ) const;

    bool EnsureFormulaCellResults( SCROW nRow1, SCROW nRow2, bool bSkipRunning = false );

    void StoreToCache(SvStream& rStrm) const;
    void RestoreFromCache(SvStream& rStrm);

#if DUMP_COLUMN_STORAGE
    void DumpColumnStorage() const;
#endif

    SCSIZE      GetPatternCount() const;
    SCSIZE      GetPatternCount( SCROW nRow1, SCROW nRow2 ) const;
    bool        ReservePatternCount( SCSIZE nReserve );
private:

    sc::CellStoreType::iterator GetPositionToInsert( SCROW nRow, std::vector<SCROW>& rNewSharedRows );
    sc::CellStoreType::iterator GetPositionToInsert( const sc::CellStoreType::iterator& it, SCROW nRow,
                                                     std::vector<SCROW>& rNewSharedRows );

    void AttachNewFormulaCell(
        const sc::CellStoreType::iterator& itPos, SCROW nRow, ScFormulaCell& rCell,
        const std::vector<SCROW>& rNewSharedRows,
        bool bJoin = true, sc::StartListeningType eListenType = sc::SingleCellListening );

    void AttachNewFormulaCell(
        const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell,
        const std::vector<SCROW>& rNewSharedRows,
        bool bJoin = true, sc::StartListeningType eListenType = sc::SingleCellListening );

    void AttachNewFormulaCells( const sc::CellStoreType::position_type& aPos, size_t nLength,
                                std::vector<SCROW>& rNewSharedRows );

    void BroadcastNewCell( SCROW nRow );
    bool UpdateScriptType( sc::CellTextAttr& rAttr, SCROW nRow, sc::CellStoreType::iterator& itr );

    const ScFormulaCell* FetchFormulaCell( SCROW nRow ) const;

    SCROW FindNextVisibleRowWithContent(
        sc::CellStoreType::const_iterator& itPos, SCROW nRow, bool bForward) const;
    SCROW FindNextVisibleRow(SCROW nRow, bool bForward) const;

    /**
     * Called whenever the state of cell array gets modified i.e. new cell
     * insertion, cell removal or relocation, cell value update and so on.
     *
     * Call this only from those methods where maCells is modified directly.
     */
    void CellStorageModified();
    void CellNotesDeleting(SCROW nRow1, SCROW nRow2, bool bForgetCaptionOwnership );

    void CopyCellTextAttrsToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol) const;

    void DeleteCells(
        sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2, InsertDeleteFlags nDelFlag,
        sc::SingleColumnSpanSet& rDeleted );

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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
