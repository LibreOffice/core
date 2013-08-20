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

#ifndef SC_COLUMN_HXX
#define SC_COLUMN_HXX

#include "markarr.hxx"
#include "global.hxx"
#include "address.hxx"
#include "rangenam.hxx"
#include "types.hxx"
#include "mtvelements.hxx"
#include "formula/types.hxx"

#include <set>
#include <vector>

#include <boost/intrusive_ptr.hpp>
#include <mdds/flat_segment_tree.hpp>

namespace editeng { class SvxBorderLine; }

namespace sc {
    struct FormulaGroupContext;
    class StartListeningContext;
    class EndListeningContext;
    class CopyFromClipContext;
    class CopyToClipContext;
    class CopyToDocContext;
    class MixDocContext;
    class ColumnSpanSet;
    struct ColumnBlockPosition;
    class SingleColumnSpanSet;
    struct RefUpdateContext;
    struct RefUpdateInsertTabContext;
    struct RefUpdateDeleteTabContext;
    struct RefUpdateMoveTabContext;
    class EditTextIterator;
}

class Fraction;
class OutputDevice;
class SfxItemPoolCache;
class SvtListener;
class SfxPoolItem;
class SfxStyleSheetBase;
class SvxBoxInfoItem;
class SvxBoxItem;

class ScAttrIterator;
class ScAttrArray;
struct ScAttrEntry;
class ScDocument;
class ScEditDataArray;
class ScFormulaCell;
class ScMarkData;
class ScPatternAttr;
class ScStyleSheet;
class SvtBroadcaster;
class ScTypedStrData;
class ScProgress;
struct ScFunctionData;
struct ScLineFlags;
struct ScMergePatternState;
class ScFlatBoolRowSegments;
struct ScSetStringParam;
struct ScColWidthParam;
class ScColumnTextWidthIterator;
struct ScFormulaCellGroup;
struct ScRefCellValue;
struct ScCellValue;
class ScDocumentImport;

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
    // type value or SC_SCRIPTTYPE_UNKNOWN.
    sc::CellTextAttrStoreType maCellTextAttrs;

    // Broadcasters for formula cells.
    sc::BroadcasterStoreType maBroadcasters;

    // Cell values.
    sc::CellStoreType maCells;

    SCCOL           nCol;
    SCTAB           nTab;

    ScAttrArray*          pAttrArray;
    ScDocument*           pDocument;
    bool mbDirtyGroups;     /// formula groups are dirty.

friend class ScDocument;                    // for FillInfo
friend class ScTable;
friend class ScValueIterator;
friend class ScHorizontalValueIterator;
friend class ScDBQueryDataIterator;
friend class ScQueryCellIterator;
friend class ScCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScColumnTextWidthIterator;
friend class ScDocumentImport;
friend class sc::SingleColumnSpanSet;
friend class sc::ColumnSpanSet;
friend class sc::EditTextIterator;

    ScColumn(const ScColumn&); // disabled
    ScColumn& operator= (const ScColumn&); // disabled

    bool ParseString(
        ScCellValue& rCell,
        SCROW nRow, SCTAB nTab, const String& rString, formula::FormulaGrammar::AddressConvention eConv,
        ScSetStringParam* pParam );

public:
                ScColumn();
                ~ScColumn();

    void        Init(SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc);

    ScDocument& GetDoc();
    const ScDocument& GetDoc() const;
    SCTAB GetTab() const { return nTab; }
    SCCOL GetCol() const { return nCol; }
    sc::CellStoreType& GetCellStore() { return maCells; }
    const sc::CellStoreType& GetCellStore() const { return maCells; }
    sc::CellTextAttrStoreType& GetCellAttrStore() { return maCellTextAttrs; }
    const sc::CellTextAttrStoreType& GetCellAttrStore() const { return maCellTextAttrs; }

    ScRefCellValue GetCellValue( SCROW nRow ) const;
    ScRefCellValue GetCellValue( const sc::CellStoreType::const_iterator& itPos, size_t nOffset ) const;

    void        Delete( SCROW nRow );
    void        FreeAll();
    void        SwapRow( SCROW nRow1, SCROW nRow2 );
    void        SwapCell( SCROW nRow, ScColumn& rCol);

    bool        HasAttrib( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const;
    bool    HasAttribSelection( const ScMarkData& rMark, sal_uInt16 nMask ) const;
    bool    ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                bool bRefresh );

    bool               IsEmptyData() const;
    bool               IsEmptyAttr() const;
    bool               IsEmpty() const;

                // data only:
    bool               IsEmptyBlock(SCROW nStartRow, SCROW nEndRow) const;
    SCSIZE         GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const;
    bool               HasDataAt(SCROW nRow) const;
    bool               HasVisibleDataAt(SCROW nRow) const;
    SCROW              GetFirstDataPos() const;
    SCROW              GetLastDataPos() const;
    bool               GetPrevDataPos(SCROW& rRow) const;
    bool               GetNextDataPos(SCROW& rRow) const;
    void               FindDataAreaPos(SCROW& rRow, bool bDown) const; // (without Broadcaster)
    void FindUsed( SCROW nStartRow, SCROW nEndRow, mdds::flat_segment_tree<SCROW, bool>& rUsed ) const;

    SCSIZE             VisibleCount( SCROW nStartRow, SCROW nEndRow ) const;
    sal_uInt16  GetBlockMatrixEdges( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const;
    bool    HasSelectionMatrixFragment(const ScMarkData& rMark) const;

    bool    GetFirstVisibleAttr( SCROW& rFirstRow ) const;
    bool    GetLastVisibleAttr( SCROW& rLastRow, bool bFullFormattedArea = false ) const;
    bool    HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const;
    bool    IsVisibleAttrEqual( const ScColumn& rCol, SCROW nStartRow = 0,
                                    SCROW nEndRow = MAXROW ) const;
    bool    IsAllAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const;

    bool    TestInsertCol( SCROW nStartRow, SCROW nEndRow) const;
    bool TestInsertRow( SCROW nStartRow, SCSIZE nSize ) const;
    void        InsertRow( SCROW nStartRow, SCSIZE nSize );
    void        DeleteRow( SCROW nStartRow, SCSIZE nSize );
    void        DeleteArea(SCROW nStartRow, SCROW nEndRow, sal_uInt16 nDelFlag );
    void CopyToClip(
        sc::CopyToClipContext& rCxt, SCROW nRow1, SCROW nRow2, ScColumn& rColumn ) const;
    void CopyStaticToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol);
    void CopyCellToDocument( SCROW nSrcRow, SCROW nDestRow, ScColumn& rDestCol );
    bool InitBlockPosition( sc::ColumnBlockPosition& rBlockPos );
    bool InitBlockPosition( sc::ColumnBlockConstPosition& rBlockPos ) const;
    void CopyFromClip(
        sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2, long nDy, ScColumn& rColumn );

    void StartListeningInArea( sc::StartListeningContext& rCxt, SCROW nRow1, SCROW nRow2 );

    void        RemoveEditAttribs( SCROW nStartRow, SCROW nEndRow );

                //  Selection (?) of this document
    void MixMarked(
        sc::MixDocContext& rCxt, const ScMarkData& rMark, sal_uInt16 nFunction,
        bool bSkipEmpty, const ScColumn& rSrcCol );
    void MixData(
        sc::MixDocContext& rCxt, SCROW nRow1, SCROW nRow2, sal_uInt16 nFunction, bool bSkipEmpty,
        const ScColumn& rSrcCol );

    ScAttrIterator* CreateAttrIterator( SCROW nStartRow, SCROW nEndRow ) const;

                //     UpdateSelectionFunction: multi-select
    void UpdateSelectionFunction(
        const ScMarkData& rMark, ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows,
        bool bDoExclude, SCROW nExStartRow, SCROW nExEndRow );

    void UpdateAreaFunction(
        ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows, SCROW nStartRow, SCROW nEndRow);

    void CopyToColumn(
        sc::CopyToDocContext& rCxt, SCROW nRow1, SCROW nRow2, sal_uInt16 nFlags, bool bMarked,
        ScColumn& rColumn, const ScMarkData* pMarkData = NULL, bool bAsLink = false) const;

    void UndoToColumn(
        sc::CopyToDocContext& rCxt, SCROW nRow1, SCROW nRow2, sal_uInt16 nFlags, bool bMarked,
        ScColumn& rColumn, const ScMarkData* pMarkData = NULL) const;

    void        CopyScenarioFrom( const ScColumn& rSrcCol );
    void        CopyScenarioTo( ScColumn& rDestCol ) const;
    bool    TestCopyScenarioTo( const ScColumn& rDestCol ) const;
    void        MarkScenarioIn( ScMarkData& rDestMark ) const;

    void        CopyUpdated( const ScColumn& rPosCol, ScColumn& rDestCol ) const;

    void        SwapCol(ScColumn& rCol);
    void        MoveTo(SCROW nStartRow, SCROW nEndRow, ScColumn& rCol);

    void MarkSubTotalCells( sc::ColumnSpanSet& rSet, SCROW nRow1, SCROW nRow2, bool bVal ) const;

    bool HasEditCells(SCROW nStartRow, SCROW nEndRow, SCROW& rFirst);

    bool SetString(
        SCROW nRow, SCTAB nTab, const String& rString, formula::FormulaGrammar::AddressConvention eConv,
        ScSetStringParam* pParam = NULL );

    void SetEditText( SCROW nRow, EditTextObject* pEditText );
    void SetEditText( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, EditTextObject* pEditText );
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
    ScFormulaCell* SetFormulaCell( SCROW nRow, ScFormulaCell* pCell );
    ScFormulaCell* SetFormulaCell( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, ScFormulaCell* pCell );
    bool SetGroupFormulaCell( SCROW nRow, ScFormulaCell* pCell );

    void SetRawString( SCROW nRow, const OUString& rStr, bool bBroadcast = true );
    void SetRawString( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, const OUString& rStr, bool bBroadcast = true );
    void SetValue( SCROW nRow, double fVal );
    void SetValue( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, double fVal, bool bBroadcast = true );
    void        SetError( SCROW nRow, const sal_uInt16 nError);

    void        GetString( SCROW nRow, OUString& rString ) const;
    const OUString* GetStringCell( SCROW nRow ) const;
    double* GetValueCell( SCROW nRow );
    void        GetInputString( SCROW nRow, OUString& rString ) const;
    double      GetValue( SCROW nRow ) const;
    const EditTextObject* GetEditText( SCROW nRow ) const;
    void RemoveEditTextCharAttribs( SCROW nRow, const ScPatternAttr& rAttr );
    void        GetFormula( SCROW nRow, OUString& rFormula ) const;
    const ScTokenArray* GetFormulaTokens( SCROW nRow ) const;
    const ScFormulaCell* GetFormulaCell( SCROW nRow ) const;
    ScFormulaCell* GetFormulaCell( SCROW nRow );
    CellType    GetCellType( SCROW nRow ) const;
    SCSIZE      GetCellCount() const;
    sal_uInt32 GetWeightedCount() const;
    sal_uInt32 GetCodeCount() const;       // RPN-Code in formulas
    sal_uInt16  GetErrCode( SCROW nRow ) const;

    bool    HasStringData( SCROW nRow ) const;
    bool    HasValueData( SCROW nRow ) const;
    bool    HasStringCells( SCROW nStartRow, SCROW nEndRow ) const;

    bool IsFormulaDirty( SCROW nRow ) const;

    void        SetDirty();
    void        SetDirty( SCROW nRow1, SCROW nRow2 );
    void        SetDirtyVar();
    void        SetDirtyAfterLoad();
    void        SetTableOpDirty( const ScRange& );
    void        CalcAll();
    void        CalcAfterLoad();
    void        CompileAll();
    void        CompileXML( ScProgress& rProgress );

    bool CompileErrorCells(sal_uInt16 nErrCode);

    void        ResetChanged( SCROW nStartRow, SCROW nEndRow );

    bool UpdateReferenceOnCopy( const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc = NULL );

    /**
     * Update reference addresses in formula cell in response to mass cell
     * movement.
     *
     * @return true if reference of at least one formula cell has been
     *         updated, false otherwise.
     */
    bool UpdateReference( sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc = NULL );

    void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt );
    void UpdateInsertTabOnlyCells( sc::RefUpdateInsertTabContext& rCxt );
    void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt );
    void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt, SCTAB nTabNo );
    void        UpdateCompile( bool bForceIfNameInUse = false );
    void        UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc );
    void        UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void        SetTabNo(SCTAB nNewTab);
    void        FindRangeNamesInUse(SCROW nRow1, SCROW nRow2, std::set<sal_uInt16>& rIndexes) const;

    const SfxPoolItem*      GetAttr( SCROW nRow, sal_uInt16 nWhich ) const;
    const ScPatternAttr*    GetPattern( SCROW nRow ) const;
    const ScPatternAttr*    GetMostUsedPattern( SCROW nStartRow, SCROW nEndRow ) const;

    sal_uInt32 GetNumberFormat( SCROW nRow ) const;
    sal_uInt32  GetNumberFormat( SCROW nStartRow, SCROW nEndRow ) const;

    void        MergeSelectionPattern( ScMergePatternState& rState, const ScMarkData& rMark, bool bDeep ) const;
    void        MergePatternArea( ScMergePatternState& rState, SCROW nRow1, SCROW nRow2, bool bDeep ) const;
    void        MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                            ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight ) const;
    void        ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight );

    void        ApplyAttr( SCROW nRow, const SfxPoolItem& rAttr );
    void        ApplyPattern( SCROW nRow, const ScPatternAttr& rPatAttr );
    void        ApplyPatternArea( SCROW nStartRow, SCROW nEndRow, const ScPatternAttr& rPatAttr,
                                  ScEditDataArray* pDataArray = NULL );
    bool        SetAttrEntries(ScAttrEntry* pData, SCSIZE nSize);
    void        SetPattern( SCROW nRow, const ScPatternAttr& rPatAttr, bool bPutToPool = false );
    void        SetPatternArea( SCROW nStartRow, SCROW nEndRow,
                                const ScPatternAttr& rPatAttr, bool bPutToPool = false );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, short nNewType );

    void        ApplyStyle( SCROW nRow, const ScStyleSheet& rStyle );
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
    bool    IsStyleSheetUsed( const ScStyleSheet& rStyle, bool bGatherAllStyles ) const;

                /// May return -1 if not found
    SCsROW SearchStyle(
        SCsROW nRow, const ScStyleSheet* pSearchStyle, bool bUp, bool bInSelection,
        const ScMarkData& rMark) const;

    bool SearchStyleRange(
        SCsROW& rRow, SCsROW& rEndRow, const ScStyleSheet* pSearchStyle, bool bUp,
        bool bInSelection, const ScMarkData& rMark) const;

    bool    ApplyFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags );
    bool    RemoveFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags );
    void        ClearItems( SCROW nStartRow, SCROW nEndRow, const sal_uInt16* pWhich );

    void        RemoveProtected( SCROW nStartRow, SCROW nEndRow );

    SCsROW      ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark, ScEditDataArray* pDataArray = NULL );
    void        DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark );

    void        ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark );
    void        ChangeSelectionIndent( bool bIncrement, const ScMarkData& rMark );

    long GetNeededSize(
        SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY,
        const Fraction& rZoomX, const Fraction& rZoomY,
        bool bWidth, const ScNeededSizeOptions& rOptions) const;

    sal_uInt16 GetOptimalColWidth(
        OutputDevice* pDev, double nPPTX, double nPPTY,
        const Fraction& rZoomX, const Fraction& rZoomY,
        bool bFormula, sal_uInt16 nOldWidth, const ScMarkData* pMarkData, const ScColWidthParam* pParam) const;

    void GetOptimalHeight(
        SCROW nStartRow, SCROW nEndRow, sal_uInt16* pHeight, OutputDevice* pDev,
        double nPPTX, double nPPTY, const Fraction& rZoomX, const Fraction& rZoomY,
        bool bShrink, sal_uInt16 nMinHeight, SCROW nMinStart);

                /// Including current, may return -1
    SCsROW      GetNextUnprotected( SCROW nRow, bool bUp ) const;

    void GetFilterEntries(SCROW nStartRow, SCROW nEndRow, std::vector<ScTypedStrData>& rStrings, bool& rHasDates);
    bool GetDataEntries( SCROW nRow, std::set<ScTypedStrData>& rStrings, bool bLimit ) const;

    void UpdateInsertTabAbs(SCTAB nNewPos);
    bool    TestTabRefAbs(SCTAB nTable) const;
    bool    GetNextSpellingCell(SCROW& nRow, bool bInSel, const ScMarkData& rData) const;

    void        RemoveAutoSpellObj();

    void        StartListening( SvtListener& rLst, SCROW nRow );
    void        EndListening( SvtListener& rLst, SCROW nRow );
    void StartListening( sc::StartListeningContext& rCxt, SCROW nRow, SvtListener& rListener );
    void EndListening( sc::EndListeningContext& rCxt, SCROW nRow, SvtListener& rListener );
    void        MoveListeners( SvtBroadcaster& rSource, SCROW nDestRow );
    void        StartAllListeners();
    void        StartNeededListeners(); // only for cells where NeedsListening()==true
    void        SetRelNameDirty();
    void BroadcastRecalcOnRefMove();

    void        CompileDBFormula();
    void        CompileDBFormula( bool bCreateFormulaString );
    void        CompileNameFormula( bool bCreateFormulaString );
    void        CompileColRowNameFormula();

    sal_Int32   GetMaxStringLen( SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const;
    xub_StrLen  GetMaxNumberStringLen( sal_uInt16& nPrecision,
                                       SCROW nRowStart, SCROW nRowEnd ) const;

    sal_uInt16 GetTextWidth(SCROW nRow) const;
    void SetTextWidth(SCROW nRow, sal_uInt16 nWidth);

    sal_uInt8 GetScriptType( SCROW nRow ) const;

    /**
     * Get combined script types of the specified range. This method may
     * update script types on demand if they have not been determined.
     */
    sal_uInt8 GetRangeScriptType( sc::CellTextAttrStoreType::iterator& itPos, SCROW nRow1, SCROW nRow2 );

    void SetScriptType( SCROW nRow, sal_uInt8 nType );

    size_t GetFormulaHash( SCROW nRow ) const;

    ScFormulaVectorState GetFormulaVectorState( SCROW nRow ) const;
    formula::FormulaTokenRef ResolveStaticReference( SCROW nRow );
    bool ResolveStaticReference( ScMatrix& rMat, SCCOL nMatCol, SCROW nRow1, SCROW nRow2 );
    void FillMatrix( ScMatrix& rMat, size_t nMatCol, SCROW nRow1, SCROW nRow2 ) const;
    const double* FetchDoubleArray( sc::FormulaGroupContext& rCxt, SCROW nRow1, SCROW nRow2 );
    void SetFormulaResults( SCROW nRow, const double* pResults, size_t nLen );

    void SetNumberFormat( SCROW nRow, sal_uInt32 nNumberFormat );

    SvtBroadcaster* GetBroadcaster( SCROW nRow );
    const SvtBroadcaster* GetBroadcaster( SCROW nRow ) const;
    void DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2 );
    bool HasBroadcaster() const;

    void Broadcast( SCROW nRow );
    void BroadcastCells( const std::vector<SCROW>& rRows );

    void InterpretDirtyCells( SCROW nRow1, SCROW nRow2 );

    void JoinNewFormulaCell( const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell ) const;

    /**
     * Detach a formula cell that's about to be deleted, or removed from
     * document storage (if that ever happens).
     */
    void DetachFormulaCell( const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell );

    void DetachFormulaCells( const sc::CellStoreType::position_type& aPos, size_t nLength );

    /**
     * Regroup formula cells for the entire column.
     */
    void RegroupFormulaCells();

#if DEBUG_COLUMN_STORAGE
    void DumpFormulaGroups() const;
#endif

private:

    sc::CellStoreType::iterator GetPositionToInsert( SCROW nRow );
    sc::CellStoreType::iterator GetPositionToInsert( const sc::CellStoreType::iterator& it, SCROW nRow );
    void ActivateNewFormulaCell( const sc::CellStoreType::iterator& itPos, SCROW nRow, ScFormulaCell& rCell, bool bJoin = true );
    void ActivateNewFormulaCell( const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell, bool bJoin = true );
    void BroadcastNewCell( SCROW nRow );
    bool UpdateScriptType( sc::CellTextAttr& rAttr, SCROW nRow );

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

    void CopyCellTextAttrsToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol) const;

    void SwapCellTextAttrs( SCROW nRow1, SCROW nRow2 );
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
