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

namespace editeng { class SvxBorderLine; }

namespace sc {
    struct FormulaGroupContext;
    class StartListeningContext;
    class EndListeningContext;
    class CopyFromClipContext;
    class CopyToClipContext;
    class CopyToDocContext;
    class MixDocContext;
    struct ColumnBlockPosition;
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
class ScBaseCell;
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

struct ColEntry
{
    SCROW       nRow;
    ScBaseCell* pCell;

    struct Less : std::binary_function<ColEntry, ColEntry, bool>
    {
        bool operator() (const ColEntry& r1, const ColEntry& r2) const;
    };
};

struct ColDoubleEntry
{
    SCROW               mnStart;
    std::vector<double> maData;

    struct LessByPtr : std::binary_function<ColDoubleEntry*, ColDoubleEntry*, bool>
    {
        bool operator() (const ColDoubleEntry* p1, const ColDoubleEntry* p2) const;
    };
};

class ScColumn
{
    // Empty values correspond with empty cells. All non-empty cell positions
    // must have non-empty elements. For text width, the value should be
    // either the real text width, or TEXTWIDTH_DIRTY in case it hasn't been
    // calculated yet. For script type, it should be either the real script
    // type value or SC_SCRIPTTYPE_UNKNOWN.
    sc::CellTextAttrStoreType maCellTextAttrs;

    sc::BroadcasterStoreType maBroadcasters;

    SCCOL           nCol;
    SCTAB           nTab;

    std::vector<ColEntry> maItems;

    // temporary until we switch to mdds container
    std::vector<ColDoubleEntry *> maDoubles;
    std::vector<ScFormulaCellGroupRef> maFnGroups;

    ScAttrArray*          pAttrArray;
    ScDocument*           pDocument;
    bool mbDirtyGroups;     /// formula groups are dirty.

friend class ScDocument;                    // for FillInfo
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
friend class ScColumnTextWidthIterator;
friend class ScDocumentImport;

    ScColumn(const ScColumn&); // disabled
    ScColumn& operator= (const ScColumn&); // disabled

    std::vector<ColEntry>::iterator Search( SCROW nRow );
    std::vector<ColEntry>::const_iterator Search( SCROW nRow ) const;

public:
                ScColumn();
                ~ScColumn();

    void        Init(SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc);

    bool    Search( SCROW nRow, SCSIZE& nIndex ) const;
    ScBaseCell* GetCell( SCROW nRow ) const;
    ScRefCellValue GetCellValue( SCROW nRow ) const;
    void Insert( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, ScBaseCell* pCell );
    void        Insert( SCROW nRow, ScBaseCell* pCell );
    void        Insert( SCROW nRow, sal_uInt32 nFormatIndex, ScBaseCell* pCell );
    void Append( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, ScBaseCell* pCell );
    void        Append( SCROW nRow, ScBaseCell* pCell );
    void        Delete( SCROW nRow );
    void        DeleteAtIndex( SCSIZE nIndex );
    void        FreeAll();
    void ReserveSize( SCSIZE nSize );
    void        SwapRow( SCROW nRow1, SCROW nRow2 );
    void        SwapCell( SCROW nRow, ScColumn& rCol);
    void        RebuildFormulaGroups();

    bool        HasAttrib( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const;
    bool    HasAttribSelection( const ScMarkData& rMark, sal_uInt16 nMask ) const;
    bool    ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                bool bRefresh );

    bool               IsEmptyVisData() const;              // without Broadcaster
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
    SCROW              GetLastVisDataPos() const;                           // without Broadcaster
    SCROW              GetFirstVisDataPos() const;
    bool               GetPrevDataPos(SCROW& rRow) const;
    bool               GetNextDataPos(SCROW& rRow) const;
    void               FindDataAreaPos(SCROW& rRow, bool bDown) const; // (without Broadcaster)
    void               FindUsed( SCROW nStartRow, SCROW nEndRow, bool* pUsed ) const;

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
    bool    TestInsertRow( SCSIZE nSize ) const;
    void        InsertRow( SCROW nStartRow, SCSIZE nSize );
    void        DeleteRow( SCROW nStartRow, SCSIZE nSize );
    void        DeleteArea(SCROW nStartRow, SCROW nEndRow, sal_uInt16 nDelFlag );
    void CopyToClip(
        sc::CopyToClipContext& rCxt, SCROW nRow1, SCROW nRow2, ScColumn& rColumn ) const;
    void CopyStaticToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol);
    void CopyCellToDocument( SCROW nSrcRow, SCROW nDestRow, ScColumn& rDestCol );
    bool InitBlockPosition( sc::ColumnBlockPosition& rBlockPos );
    void CopyFromClip(
        sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2, long nDy, ScColumn& rColumn );

    void StartListeningInArea( sc::StartListeningContext& rCxt, SCROW nRow1, SCROW nRow2 );
    void        BroadcastInArea( SCROW nRow1, SCROW nRow2 );

    void        RemoveEditAttribs( SCROW nStartRow, SCROW nEndRow );

                //  Selection (?) of this document
    void MixMarked(
        sc::MixDocContext& rCxt, const ScMarkData& rMark, sal_uInt16 nFunction,
        bool bSkipEmpty, const ScColumn& rSrcCol );
    void MixData(
        sc::MixDocContext& rCxt, SCROW nRow1, SCROW nRow2, sal_uInt16 nFunction, bool bSkipEmpty,
        const ScColumn& rSrcCol );

    ScFormulaCell*  CreateRefCell( ScDocument* pDestDoc, const ScAddress& rDestPos,
                                    SCSIZE nIndex, sal_uInt16 nFlags ) const;

    ScAttrIterator* CreateAttrIterator( SCROW nStartRow, SCROW nEndRow ) const;


    SCCOL              GetCol() const { return nCol; }

                //     UpdateSelectionFunction: multi-select
    void UpdateSelectionFunction(
        const ScMarkData& rMark, ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows,
        bool bDoExclude, SCROW nExStartRow, SCROW nExEndRow ) const;

    void UpdateAreaFunction(
        ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows, SCROW nStartRow, SCROW nEndRow) const;

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


    bool HasEditCells(SCROW nStartRow, SCROW nEndRow, SCROW& rFirst);

    bool SetString(
        SCROW nRow, SCTAB nTab, const String& rString, formula::FormulaGrammar::AddressConvention eConv,
        ScSetStringParam* pParam = NULL );

    void SetEditText( SCROW nRow, EditTextObject* pEditText );
    void SetEditText( SCROW nRow, const EditTextObject& rEditText, const SfxItemPool* pEditPool );
    void SetFormula( SCROW nRow, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram );
    void SetFormula( SCROW nRow, const OUString& rFormula, formula::FormulaGrammar::Grammar eGram );
    void SetFormulaCell( SCROW nRow, ScFormulaCell* pCell );

    void        SetValue( SCROW nRow, const double& rVal);
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
    void        SetDirty( const ScRange& );
    void        SetDirtyVar();
    void        SetDirtyAfterLoad();
    void        SetTableOpDirty( const ScRange& );
    void        CalcAll();
    void        CalcAfterLoad();
    void        CompileAll();
    void        CompileXML( ScProgress& rProgress );

    bool CompileErrorCells(sal_uInt16 nErrCode);

    void        ResetChanged( SCROW nStartRow, SCROW nEndRow );

    bool        UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                     SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                     ScDocument* pUndoDoc = NULL );
    void UpdateInsertTab(SCTAB nInsPos, SCTAB nNewSheets = 1);
    void UpdateInsertTabOnlyCells(SCTAB nInsPos, SCTAB nNewSheets = 1);
    void UpdateDeleteTab(SCTAB nDelPos, bool bIsMove, ScColumn* pRefUndo = NULL, SCTAB nSheets = 1);
    void        UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo);
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
    bool GetDataEntries(SCROW nRow, std::set<ScTypedStrData>& rStrings, bool bLimit);

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
    sal_uInt8 GetRangeScriptType( sc::CellTextAttrStoreType::iterator& itPos, SCROW nRow1, SCROW nRow2 );

    void SetScriptType( SCROW nRow, sal_uInt8 nType );

    size_t GetFormulaHash( SCROW nRow ) const;

    ScFormulaVectorState GetFormulaVectorState( SCROW nRow ) const;
    formula::FormulaTokenRef ResolveStaticReference( SCROW nRow );
    bool ResolveStaticReference( ScMatrix& rMat, SCCOL nMatCol, SCROW nRow1, SCROW nRow2 );
    const double* FetchDoubleArray( sc::FormulaGroupContext& rCxt, SCROW nRow1, SCROW nRow2 ) const;

    ScRefCellValue GetRefCellValue( SCROW );

    void SetNumberFormat( SCROW nRow, sal_uInt32 nNumberFormat );

    SvtBroadcaster* GetBroadcaster( SCROW nRow );
    const SvtBroadcaster* GetBroadcaster( SCROW nRow ) const;
    void DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2 );
    bool HasBroadcaster() const;

private:
    void UpdateScriptType( sc::CellTextAttr& rAttr, SCROW nRow );

    void DeleteRange(
        SCSIZE nStartIndex, SCSIZE nEndIndex, sal_uInt16 nDelFlag, std::vector<SCROW>& rDeletedRows );

    const ScFormulaCell* FetchFormulaCell( SCROW nRow ) const;

    ScBaseCell* CloneCell(SCSIZE nIndex, sal_uInt16 nFlags, ScDocument& rDestDoc, const ScAddress& rDestPos) const;

    SCROW FindNextVisibleRowWithContent(SCROW nRow, bool bForward) const;
    SCROW FindNextVisibleRow(SCROW nRow, bool bForward) const;

    /**
     * Called whenever the state of cell array gets modified i.e. new cell
     * insertion, cell removal or relocation, cell value update and so on.
     *
     * Call this only from those methods where maItems is modified directly.
     */
    void CellStorageModified();

    void CopyCellTextAttrsToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol) const;

    void SetCell( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, ScBaseCell* pNewCell );
    void SetCell( SCROW nRow, ScBaseCell* pNewCell );
    void PostSetCell( SCROW nRow, ScBaseCell* pNewCell );

    /**
     * Clear and re-populate the cell text attribute array from the non-empty
     * cells stored in the cell array.
     */
    void ResetCellTextAttrs();
};


class ScColumnIterator                                 // walk through all data of a area/range
{
    const ScColumn*            pColumn;
    SCSIZE                             nPos;
    SCROW               nTop;
    SCROW               nBottom;
public:
                ScColumnIterator( const ScColumn* pCol, SCROW nStart=0, SCROW nEnd=MAXROW );
                ~ScColumnIterator();

    bool    Next( SCROW& rRow, ScBaseCell*& rpCell );
    SCSIZE      GetIndex() const;
};


class ScMarkedDataIter                                 // walk through data in a selected area/range
{
    const ScColumn*            pColumn;
    SCSIZE                             nPos;
    ScMarkArrayIter*    pMarkIter;
    SCROW               nTop;
    SCROW               nBottom;
    bool            bNext;
    bool            bAll;

public:
                ScMarkedDataIter( const ScColumn* pCol, const ScMarkData* pMarkData,
                                    bool bAllIfNone = false );
                ~ScMarkedDataIter();

    bool    Next( SCSIZE& rIndex );
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
