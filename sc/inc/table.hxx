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

#ifndef SC_TABLE_HXX
#define SC_TABLE_HXX

#include <vector>
#include <utility>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "column.hxx"
#include "sortparam.hxx"
#include "compressedarray.hxx"
#include "postit.hxx"
#include "types.hxx"
#include "cellvalue.hxx"
#include "formula/types.hxx"
#include "calcmacros.hxx"

#include <set>
#include <map>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace utl {
    class TextSearch;
}

namespace com { namespace sun { namespace star {
    namespace sheet {
        struct TablePageBreakData;
    }
} } }

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
    struct RefUpdateContext;
    struct RefUpdateInsertTabContext;
    struct RefUpdateDeleteTabContext;
    struct RefUpdateMoveTabContext;
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

class ScTable : boost::noncopyable
{
private:
    typedef ::std::vector< ScRange > ScRangeVec;
    typedef ::std::pair< SCCOL, SCROW > ScAddress2D;
    typedef ::std::vector< ScAddress2D > ScAddress2DVec;

    ScColumn        aCol[MAXCOLCOUNT];

    OUString aName;
    OUString aCodeName;
    OUString aComment;

    OUString       aLinkDoc;
    OUString       aLinkFlt;
    OUString       aLinkOpt;
    OUString       aLinkTab;
    sal_uLong           nLinkRefreshDelay;
    sal_uInt8           nLinkMode;

    // page style template
    OUString   aPageStyle;
    Size            aPageSizeTwips;                 // size of the print-page
    SCCOL           nRepeatStartX;                  // repeating rows/columns
    SCCOL           nRepeatEndX;                    // REPEAT_NONE, if not used
    SCROW           nRepeatStartY;
    SCROW           nRepeatEndY;

    boost::scoped_ptr<ScTableProtection> pTabProtection;

    sal_uInt16*         pColWidth;
    boost::scoped_ptr<ScFlatUInt16RowSegments> mpRowHeights;

    sal_uInt8*          pColFlags;
    ScBitMaskCompressedArray< SCROW, sal_uInt8>*     pRowFlags;
    boost::scoped_ptr<ScFlatBoolColSegments>  mpHiddenCols;
    boost::scoped_ptr<ScFlatBoolRowSegments>  mpHiddenRows;
    boost::scoped_ptr<ScFlatBoolColSegments>  mpFilteredCols;
    boost::scoped_ptr<ScFlatBoolRowSegments>  mpFilteredRows;

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

    boost::scoped_ptr<ScAddress2DVec> mxUninitNotes;

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
    sal_uInt16          nScenarioFlags;
    ScDBData*       pDBDataNoName;
    mutable ScRangeName* mpRangeName;

    boost::scoped_ptr<ScConditionalFormatList> mpCondFormatList;

    ScNotes         maNotes;

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

friend class ScDocument;                    // for FillInfo
friend class ScValueIterator;
friend class ScHorizontalValueIterator;
friend class ScDBQueryDataIterator;
friend class ScCellIterator;
friend class ScQueryCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScDocAttrIterator;
friend class ScAttrRectIterator;
friend class ScColumnTextWidthIterator;
friend class ScDocumentImport;
friend class sc::ColumnSpanSet;

public:
                ScTable( ScDocument* pDoc, SCTAB nNewTab, const OUString& rNewName,
                         bool bColInfo = true, bool bRowInfo = true );
                ~ScTable();

    ScDocument& GetDoc();
    const ScDocument& GetDoc() const;
    SCTAB GetTab() const { return nTab; }

    ScOutlineTable* GetOutlineTable()               { return pOutlineTable; }

    SCSIZE      GetCellCount(SCCOL nCol) const;
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

    void MarkSubTotalCells( sc::ColumnSpanSet& rSet, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bVal ) const;

    const ScSheetEvents* GetSheetEvents() const              { return pSheetEvents; }
    void        SetSheetEvents( const ScSheetEvents* pNew );

    bool        IsVisible() const                            { return bVisible; }
    void        SetVisible( bool bVis );

    bool        IsStreamValid() const                        { return bStreamValid; }
    void        SetStreamValid( bool bSet, bool bIgnoreLock = false );

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
    void        SetScenarioColor(const Color& rNew)          { aScenarioColor = rNew; }
    const Color& GetTabBgColor() const;
    void         SetTabBgColor(const Color& rColor);
    sal_uInt16      GetScenarioFlags() const                     { return nScenarioFlags; }
    void        SetScenarioFlags(sal_uInt16 nNew)                { nScenarioFlags = nNew; }
    void        SetActiveScenario(bool bSet)                 { bActiveScenario = bSet; }
    bool        IsActiveScenario() const                     { return bActiveScenario; }

    sal_uInt8       GetLinkMode() const                         { return nLinkMode; }
    bool        IsLinked() const                            { return nLinkMode != SC_LINK_NONE; }
    const OUString& GetLinkDoc() const                        { return aLinkDoc; }
    const OUString& GetLinkFlt() const                        { return aLinkFlt; }
    const OUString& GetLinkOpt() const                        { return aLinkOpt; }
    const OUString& GetLinkTab() const                        { return aLinkTab; }
    sal_uLong       GetLinkRefreshDelay() const                 { return nLinkRefreshDelay; }

    void        SetLink( sal_uInt8 nMode, const String& rDoc, const String& rFlt,
                        const String& rOpt, const String& rTab, sal_uLong nRefreshDelay );

    void        GetName( OUString& rName ) const;
    void        SetName( const OUString& rNewName );

    void        SetAnonymousDBData(ScDBData* pDBData);
    ScDBData*   GetAnonymousDBData();

    void        GetCodeName( OUString& rName ) const {  rName = aCodeName; }
    void        SetCodeName( const OUString& rNewName ) { aCodeName = rNewName; }

    const OUString& GetUpperName() const;

    const OUString&   GetPageStyle() const                    { return aPageStyle; }
    void            SetPageStyle( const OUString& rName );
    void            PageStyleModified( const String& rNewName );

    bool            IsProtected() const;
    void            SetProtection(const ScTableProtection* pProtect);
    ScTableProtection* GetProtection();

    Size            GetPageSize() const;
    void            SetPageSize( const Size& rSize );
    void            SetRepeatArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow );

    void        RemoveAutoSpellObj();

    void        LockTable();
    void        UnlockTable();

    bool        IsBlockEditable( SCCOL nCol1, SCROW nRow1, SCCOL nCol2,
                        SCROW nRow2, bool* pOnlyNotBecauseOfMatrix = NULL ) const;
    bool        IsSelectionEditable( const ScMarkData& rMark,
                        bool* pOnlyNotBecauseOfMatrix = NULL ) const;

    bool        HasBlockMatrixFragment( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;
    bool        HasSelectionMatrixFragment( const ScMarkData& rMark ) const;

    bool        IsBlockEmpty( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bIgnoreNotes = false ) const;

    bool        SetString( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString,
                           ScSetStringParam* pParam = NULL );

    void SetEditText( SCCOL nCol, SCROW nRow, EditTextObject* pEditText );
    void SetEditText( SCCOL nCol, SCROW nRow, const EditTextObject& rEditText, const SfxItemPool* pEditPool );

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

    void        SetValue( SCCOL nCol, SCROW nRow, const double& rVal );
    void        SetError( SCCOL nCol, SCROW nRow, sal_uInt16 nError);

    void SetRawString( SCCOL nCol, SCROW nRow, const OUString& rStr );
    void        GetString( SCCOL nCol, SCROW nRow, OUString& rString ) const;
    const OUString* GetStringCell( SCCOL nCol, SCROW nRow ) const;
    double* GetValueCell( SCCOL nCol, SCROW nRow );
    void        GetInputString( SCCOL nCol, SCROW nRow, OUString& rString ) const;
    double      GetValue( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetValue( rPos.Row() ) :
                            0.0;
                    }
    double      GetValue( SCCOL nCol, SCROW nRow ) const;
    const EditTextObject* GetEditText( SCCOL nCol, SCROW nRow ) const;
    void RemoveEditTextCharAttribs( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr );
    void GetFormula( SCCOL nCol, SCROW nRow, OUString& rFormula ) const;
    const ScTokenArray* GetFormulaTokens( SCCOL nCol, SCROW nRow ) const;
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

    ScNotes*    GetNotes();
    /** Creates the captions of all uninitialized cell notes.
        @param bForced  True = always create all captions, false = skip when Undo is disabled. */
    void        InitializeNoteCaptions( bool bForced = false );

    bool TestInsertRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize ) const;
    void        InsertRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize );
    void        DeleteRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize,
                            bool* pUndoOutline = NULL );

    bool        TestInsertCol( SCROW nStartRow, SCROW nEndRow, SCSIZE nSize ) const;
    void        InsertCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize );
    void        DeleteCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize,
                            bool* pUndoOutline = NULL );

    void        DeleteArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sal_uInt16 nDelFlag);
    void CopyToClip( sc::CopyToClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScTable* pTable );
    void CopyToClip( sc::CopyToClipContext& rCxt, const ScRangeList& rRanges, ScTable* pTable );
    void CopyStaticToDocument(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScTable* pDestTab);
    void CopyCellToDocument( SCCOL nSrcCol, SCROW nSrcRow, SCCOL nDestCol, SCROW nDestRow, ScTable& rDestTab );

    bool InitColumnBlockPosition( sc::ColumnBlockPosition& rBlockPos, SCCOL nCol );

    void CopyFromClip(
        sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        SCsCOL nDx, SCsROW nDy, ScTable* pTable );

    void StartListeningInArea(
        sc::StartListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    void        BroadcastInArea( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2 );

    void CopyToTable(
        sc::CopyToDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        sal_uInt16 nFlags, bool bMarked, ScTable* pDestTab,
        const ScMarkData* pMarkData = NULL, bool bAsLink = false, bool bColRowFlags = true );

    void UndoToTable(
        sc::CopyToDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        sal_uInt16 nFlags, bool bMarked, ScTable* pDestTab, const ScMarkData* pMarkData = NULL );

    void        CopyConditionalFormat( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            SCsCOL nDx, SCsROW nDy, ScTable* pTable);
    void        TransposeClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                ScTable* pTransClip, sal_uInt16 nFlags, bool bAsLink );

                // mark of this document
    void MixMarked(
        sc::MixDocContext& rCxt, const ScMarkData& rMark, sal_uInt16 nFunction,
        bool bSkipEmpty, const ScTable* pSrcTab );

    void MixData(
        sc::MixDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        sal_uInt16 nFunction, bool bSkipEmpty, const ScTable* pSrcTab );

    void        CopyData( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            SCCOL nDestCol, SCROW nDestRow, SCTAB nDestTab );

    void        CopyScenarioFrom( const ScTable* pSrcTab );
    void        CopyScenarioTo( ScTable* pDestTab ) const;
    bool        TestCopyScenarioTo( const ScTable* pDestTab ) const;
    void        MarkScenarioIn( ScMarkData& rMark, sal_uInt16 nNeededBits ) const;
    bool        HasScenarioRange( const ScRange& rRange ) const;
    void        InvalidateScenarioRanges();
    const ScRangeList* GetScenarioRanges() const;

    void        CopyUpdated( const ScTable* pPosTab, ScTable* pDestTab ) const;

    void        InvalidateTableArea();
    void        InvalidatePageBreaks();

    bool        GetCellArea( SCCOL& rEndCol, SCROW& rEndRow ) const;            // FALSE = empty
    bool        GetTableArea( SCCOL& rEndCol, SCROW& rEndRow ) const;
    bool        GetPrintArea( SCCOL& rEndCol, SCROW& rEndRow, bool bNotes, bool bFullFormattedArea = false ) const;
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
                                      SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly ) const;

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

    sal_uInt16      GetErrCode( const ScAddress& rPos ) const
                    {
                        return ValidColRow(rPos.Col(),rPos.Row()) ?
                            aCol[rPos.Col()].GetErrCode( rPos.Row() ) :
                            0;
                    }

    bool IsEmptyData( SCCOL nCol ) const;

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

    bool CompileErrorCells(sal_uInt16 nErrCode);

    void UpdateReference(
        sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc = NULL,
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
                                 std::set<sal_uInt16>& rIndexes) const;
    void        Fill( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                        double nStepValue, double nMaxValue, ScProgress* pProgress);
    String      GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY );

    void        UpdateSelectionFunction( ScFunctionData& rData,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark );

    void        AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    sal_uInt16 nFormatNo );
    void        GetAutoFormatData(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, ScAutoFormatData& rData);
    void        ScReplaceTabsStr( String& rStr, const String& rSrch, const String& rRepl ); // from sw
    bool        SearchAndReplace(
        const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark,
        ScRangeList& rMatchedRanges, OUString& rUndoStr, ScDocument* pUndoDoc);

    void        FindMaxRotCol( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2 );

    void        GetBorderLines( SCCOL nCol, SCROW nRow,
                                const ::editeng::SvxBorderLine** ppLeft, const ::editeng::SvxBorderLine** ppTop,
                                const ::editeng::SvxBorderLine** ppRight, const ::editeng::SvxBorderLine** ppBottom ) const;

    bool        HasAttrib( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sal_uInt16 nMask ) const;
    bool        HasAttribSelection( const ScMarkData& rMark, sal_uInt16 nMask ) const;
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
                                  const ScPatternAttr& rAttr, ScEditDataArray* pDataArray = NULL );
    bool        SetAttrEntries(SCCOL nCol, ScAttrEntry* pData, SCSIZE nSize);

    void        SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr, bool bPutToPool = false )
                    {
                        if (ValidColRow(rPos.Col(),rPos.Row()))
                            aCol[rPos.Col()].SetPattern( rPos.Row(), rAttr, bPutToPool );
                    }
    void        SetPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr, bool bPutToPool = false );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, short nNewType );
    void        AddCondFormatData( const ScRangeList& rRange, sal_uInt32 nIndex );
    void        RemoveCondFormatData( const ScRangeList& rRange, sal_uInt32 nIndex );

    void        ApplyStyle( SCCOL nCol, SCROW nRow, const ScStyleSheet& rStyle );
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

    bool        IsStyleSheetUsed( const ScStyleSheet& rStyle, bool bGatherAllStyles ) const;

    bool        ApplyFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, sal_Int16 nFlags );
    bool        RemoveFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, sal_Int16 nFlags );

    void        ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark, ScEditDataArray* pDataArray = NULL );
    void        DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark );

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
    bool        SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nExtra,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    bool bForce,
                                    ScProgress* pOuterProgress = NULL, sal_uLong nProgressStart = 0 );

    void        SetOptimalHeightOnly(SCROW nStartRow, SCROW nEndRow, sal_uInt16 nExtra,
                                     OutputDevice* pDev,
                                     double nPPTX, double nPPTY,
                                     const Fraction& rZoomX, const Fraction& rZoomY,
                                     bool bForce,
                                     ScProgress* pOuterProgress = NULL, sal_uLong nProgressStart = 0 );

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
    SC_DLLPUBLIC sal_uInt16 GetRowHeight( SCROW nRow, SCROW* pStartRow = NULL, SCROW* pEndRow = NULL, bool bHiddenAsZero = true ) const;
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

    void        SetRowFlags( SCROW nRow, sal_uInt8 nNewFlags );
    void        SetRowFlags( SCROW nStartRow, SCROW nEndRow, sal_uInt8 nNewFlags );

                /// @return  the index of the last row with any set flags (auto-pagebreak is ignored).
    SCROW      GetLastFlaggedRow() const;

                /// @return  the index of the last changed column (flags and column width, auto pagebreak is ignored).
    SCCOL      GetLastChangedCol() const;
                /// @return  the index of the last changed row (flags and row height, auto pagebreak is ignored).
    SCROW      GetLastChangedRow() const;

    bool       IsDataFiltered(SCCOL nColStart, SCROW nRowStart, SCCOL nColEnd, SCROW nRowEnd) const;
    bool       IsDataFiltered(const ScRange& rRange) const;
    sal_uInt8       GetColFlags( SCCOL nCol ) const;
    sal_uInt8       GetRowFlags( SCROW nRow ) const;

    const ScBitMaskCompressedArray< SCROW, sal_uInt8> * GetRowFlagsArray() const
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
    sal_uInt32  GetTotalRowHeight(SCROW nStartRow, SCROW nEndRow, bool bHiddenAsZero = true) const;

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

    bool IsManualRowHeight(SCROW nRow) const;

    void        SyncColRowFlags();

    void        StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );
    void        ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 );

    void        Sort(const ScSortParam& rSortParam, bool bKeepQuery, ScProgress* pProgress);
    bool ValidQuery(
        SCROW nRow, const ScQueryParam& rQueryParam, ScRefCellValue* pCell = NULL,
        bool* pbTestEqualCondition = NULL);
    void        TopTenQuery( ScQueryParam& );
    SCSIZE      Query(ScQueryParam& rQueryParam, bool bKeepSub);
    bool        CreateQueryParam(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);

    void GetFilterEntries(SCCOL nCol, SCROW nRow1, SCROW nRow2, std::vector<ScTypedStrData>& rStrings, bool& rHasDates);
    void GetFilteredFilterEntries(SCCOL nCol, SCROW nRow1, SCROW nRow2, const ScQueryParam& rParam, std::vector<ScTypedStrData>& rStrings, bool& rHasDates );
    bool GetDataEntries(SCCOL nCol, SCROW nRow, std::set<ScTypedStrData>& rStrings, bool bLimit);

    bool        HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow ) const;
    bool        HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow ) const;

    sal_Int32   GetMaxStringLen( SCCOL nCol,
                                    SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const;
    xub_StrLen  GetMaxNumberStringLen( sal_uInt16& nPrecision,
                                       SCCOL nCol,
                                       SCROW nRowStart, SCROW nRowEnd ) const;

    bool        IsSortCollatorGlobal() const;
    void        InitSortCollator( const ScSortParam& rPar );
    void        DestroySortCollator();
    void        SetDrawPageSize( bool bResetStreamValid = true, bool bUpdateNoteCaptionPos = true );

    void SetRangeName(ScRangeName* pNew);
    ScRangeName* GetRangeName() const;

    ScConditionalFormatList* GetCondFormList();
    const ScConditionalFormatList* GetCondFormList() const;
    void SetCondFormList( ScConditionalFormatList* pList );

    void DeleteConditionalFormat(sal_uLong nOldIndex);

    sal_uLong          AddCondFormat( ScConditionalFormat* pNew );

    sal_uInt8 GetScriptType( SCCOL nCol, SCROW nRow ) const;
    void SetScriptType( SCCOL nCol, SCROW nRow, sal_uInt8 nType );

    sal_uInt8 GetRangeScriptType( sc::ColumnBlockPosition& rBlockPos, SCCOL nCol, SCROW nRow1, SCROW nRow2 );

    size_t GetFormulaHash( SCCOL nCol, SCROW nRow ) const;

    ScFormulaVectorState GetFormulaVectorState( SCCOL nCol, SCROW nRow ) const;
    formula::FormulaTokenRef ResolveStaticReference( SCCOL nCol, SCROW nRow );
    formula::FormulaTokenRef ResolveStaticReference( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    const double* FetchDoubleArray(
        sc::FormulaGroupContext& rCxt, SCCOL nCol, SCROW nRow1, SCROW nRow2 );

    ScRefCellValue GetRefCellValue( SCCOL nCol, SCROW nRow );

    SvtBroadcaster* GetBroadcaster( SCCOL nCol, SCROW nRow );
    const SvtBroadcaster* GetBroadcaster( SCCOL nCol, SCROW nRow ) const;
    void DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, SCCOL nCol, SCROW nRow1, SCROW nRow2 );
    bool HasBroadcaster( SCCOL nCol ) const;

    void FillMatrix( ScMatrix& rMat, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;

    void InterpretDirtyCells( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    void SetFormulaResults( SCCOL nCol, SCROW nRow, const double* pResults, size_t nLen );

    /**
     * Have formula cells with NeedsListening() == true start listening to the
     * document.
     */
    void StartNeededListeners();

    /**
     * Mark dirty those formula cells that has named ranges with relative
     * references.
     */
    void SetRelNameDirty();

    /**
     * Broadcast dirty formula cells that contain functions such as CELL(),
     * COLUMN() or ROW() which may change its value on move.
     */
    void BroadcastRecalcOnRefMove();

#if DEBUG_COLUMN_STORAGE
    void DumpFormulaGroups( SCCOL nCol ) const;
#endif

    /** Replace behaves differently to the Search; adjust the rCol and rRow accordingly.

        'Replace' replaces at the 'current' position, but in order to achieve
        that, we have to 'shift' the rCol / rRow to the 'previous' position -
        what it is depends on various settings in rSearchItem.
    */
    static void UpdateSearchItemAddressForReplace( const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow );

private:
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
    void        DecoladeRow( ScSortInfoArray*, SCROW nRow1, SCROW nRow2 );
    void        SwapCol(SCCOL nCol1, SCCOL nCol2);
    void        SwapRow(SCROW nRow1, SCROW nRow2);
    short CompareCell(
        sal_uInt16 nSort,
        ScRefCellValue& rCell1, SCCOL nCell1Col, SCROW nCell1Row,
        ScRefCellValue& rCell2, SCCOL nCell2Col, SCROW nCell2Row ) const;
    short       Compare(SCCOLROW nIndex1, SCCOLROW nIndex2) const;
    short       Compare( ScSortInfoArray*, SCCOLROW nIndex1, SCCOLROW nIndex2) const;
    ScSortInfoArray*    CreateSortInfoArray( SCCOLROW nInd1, SCCOLROW nInd2 );
    void        QuickSort( ScSortInfoArray*, SCsCOLROW nLo, SCsCOLROW nHi);
    void        SortReorder( ScSortInfoArray*, ScProgress* );

    bool        CreateExcelQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);
    bool        CreateStarQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam);
    void        GetUpperCellString(SCCOL nCol, SCROW nRow, OUString& rStr);

    bool        RefVisible(ScFormulaCell* pCell);

    bool        IsEmptyLine(SCROW nRow, SCCOL nStartCol, SCCOL nEndCol) const;

    void        IncDate(double& rVal, sal_uInt16& nDayOfMonth, double nStep, FillDateCmd eCmd);
    void        FillFormula(sal_uLong& nFormulaCounter, bool bFirst, ScFormulaCell* pSrcCell,
                            SCCOL nDestCol, SCROW nDestRow, bool bLast );
    void        UpdateInsertTabAbs(SCTAB nNewPos);
    bool        GetNextSpellingCell(SCCOL& rCol, SCROW& rRow, bool bInSel,
                                    const ScMarkData& rMark) const;
    bool        GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark ) const;
    bool        TestTabRefAbs(SCTAB nTable) const;
    void        CompileDBFormula();
    void        CompileDBFormula( bool bCreateFormulaString );
    void        CompileNameFormula( bool bCreateFormulaString );
    void        CompileColRowNameFormula();
    void        RebuildFormulaGroups();

    void        StartListening( const ScAddress& rAddress, SvtListener* pListener );
    void        EndListening( const ScAddress& rAddress, SvtListener* pListener );
    void StartListening( sc::StartListeningContext& rCxt, SCCOL nCol, SCROW nRow, SvtListener& rListener );
    void EndListening( sc::EndListeningContext& rCxt, SCCOL nCol, SCROW nRow, SvtListener& rListener );
    void        StartAllListeners();

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
        SCROW getRow() const;

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
