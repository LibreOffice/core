/*************************************************************************
 *
 *  $RCSfile: table.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-23 09:48:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_TABLE_HXX
#define SC_TABLE_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef SC_COLUMN_HXX
#include "column.hxx"
#endif
#ifndef SC_SORTPARAM_HXX
#include "sortparam.hxx"
#endif

namespace utl {
    class SearchParam;
    class TextSearch;
}

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
class ScSortInfoArray;
class ScStyleSheet;
class ScTableLink;
class ScUserListData;
class ScIndexMap;
struct RowInfo;
struct ScFunctionData;
struct ScLineFlags;
class CollatorWrapper;


class ScTable
{
private:
                                            //  Daten pro Tabelle   ------------------
    ScColumn        aCol[MAXCOL+1];

    String          aName;
    String          aComment;
    BOOL            bScenario;

    String          aLinkDoc;
    String          aLinkFlt;
    String          aLinkOpt;
    String          aLinkTab;
    BYTE            nLinkMode;

    // Seitenformatvorlage
    String          aPageStyle;
    BOOL            bPageSizeValid;
    Size            aPageSizeTwips;                 // Groesse der Druck-Seite
    USHORT          nRepeatStartX;                  // Wiederholungszeilen/Spalten
    USHORT          nRepeatEndX;                    // REPEAT_NONE, wenn nicht benutzt
    USHORT          nRepeatStartY;
    USHORT          nRepeatEndY;

    BOOL            bProtected;
    com::sun::star::uno::Sequence<sal_Int8> aProtectPass;

    USHORT*         pColWidth;
    USHORT*         pRowHeight;

    BYTE*           pColFlags;
    BYTE*           pRowFlags;

    ScOutlineTable* pOutlineTable;

    USHORT          nTableAreaX;
    USHORT          nTableAreaY;
    BOOL            bTableAreaValid;

                                            //  interne Verwaltung  ------------------
    BOOL            bVisible;

    USHORT          nTab;
    USHORT          nRecalcLvl;             // Rekursionslevel Size-Recalc
    ScDocument*     pDocument;
    utl::SearchParam*   pSearchParam;
    utl::TextSearch*    pSearchText;

    // SortierParameter um den Stackbedarf von Quicksort zu Minimieren
    ScSortParam     aSortParam;
    CollatorWrapper*    pSortCollator;
    BOOL            bGlobalKeepQuery;
    BOOL            bSharedNameInserted;

    USHORT          nPrintRangeCount;
    ScRange*        pPrintRanges;
    ScRange*        pRepeatColRange;
    ScRange*        pRepeatRowRange;

    USHORT          nLockCount;

    ScRangeList*    pScenarioRanges;
    Color           aScenarioColor;
    USHORT          nScenarioFlags;
    BOOL            bActiveScenario;

friend class ScDocument;                    // fuer FillInfo
friend class ScDocumentIterator;
friend class ScValueIterator;
friend class ScQueryValueIterator;
friend class ScCellIterator;
friend class ScQueryCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScDocAttrIterator;
friend class ScAttrRectIterator;


public:
                ScTable( ScDocument* pDoc, USHORT nNewTab, const String& rNewName,
                            BOOL bColInfo = TRUE, BOOL bRowInfo = TRUE );
                ~ScTable();

    ScOutlineTable* GetOutlineTable()               { return pOutlineTable; }

    long        GetCellCount() const;
    long        GetWeightedCount() const;
    ULONG       GetCodeCount() const;       // RPN-Code in Formeln

    BOOL        SetOutlineTable( const ScOutlineTable* pNewOutline );
    void        StartOutlineTable();

    void        DoAutoOutline( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow );

    BOOL        TestRemoveSubTotals( const ScSubTotalParam& rParam );
    void        RemoveSubTotals( ScSubTotalParam& rParam );
    BOOL        DoSubTotals( ScSubTotalParam& rParam );

    BOOL        IsVisible() const                            { return bVisible; }
    void        SetVisible( BOOL bVis );

    BOOL        IsScenario() const                           { return bScenario; }
    void        SetScenario( BOOL bFlag );
    void        GetScenarioComment( String& rComment) const  { rComment = aComment; }
    void        SetScenarioComment( const String& rComment ) { aComment = rComment; }
    const Color& GetScenarioColor() const                    { return aScenarioColor; }
    void        SetScenarioColor(const Color& rNew)          { aScenarioColor = rNew; }
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

    void        SetLink( BYTE nMode, const String& rDoc, const String& rFlt, const String& rOpt,
                            const String& rTab );

    void        GetName( String& rName ) const;
    void        SetName( const String& rNewName );

    const String&   GetPageStyle() const                    { return aPageStyle; }
    void            SetPageStyle( const String& rName );
    void            PageStyleModified( const String& rNewName );

    BOOL            IsProtected() const                     { return bProtected; }
    const com::sun::star::uno::Sequence<sal_Int8>&  GetPassword() const                     { return aProtectPass; }
    void            SetProtection( BOOL bProtect, const com::sun::star::uno::Sequence<sal_Int8>& rPasswd )
                                        { bProtected = bProtect; aProtectPass = rPasswd; }

    Size            GetPageSize() const;
    void            SetPageSize( const Size& rSize );
    void            SetRepeatArea( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nEndRow );

    void        RemoveAutoSpellObj();

    void        LockTable();
    void        UnlockTable();

    BOOL        IsBlockEditable( USHORT nCol1, USHORT nRow1, USHORT nCol2,
                        USHORT nRow2, BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL        IsSelectionEditable( const ScMarkData& rMark,
                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;

    BOOL        HasBlockMatrixFragment( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) const;
    BOOL        HasSelectionMatrixFragment( const ScMarkData& rMark ) const;

    BOOL        IsBlockEmpty( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) const;

    void        PutCell( const ScAddress&, ScBaseCell* pCell );
    void        PutCell( const ScAddress&, ULONG nFormatIndex, ScBaseCell* pCell);
    void        PutCell( USHORT nCol, USHORT nRow, ScBaseCell* pCell );
    void        PutCell(USHORT nCol, USHORT nRow, ULONG nFormatIndex, ScBaseCell* pCell);
                //  TRUE = Zahlformat gesetzt
    BOOL        SetString( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString );
    void        SetValue( USHORT nCol, USHORT nRow, const double& rVal );
    void        SetNote( USHORT nCol, USHORT nRow, const ScPostIt& rNote);
    void        SetError( USHORT nCol, USHORT nRow, USHORT nError);

    void        GetString( USHORT nCol, USHORT nRow, String& rString );
    void        GetInputString( USHORT nCol, USHORT nRow, String& rString );
    double      GetValue( const ScAddress& rPos ) const
                    { return aCol[rPos.Col()].GetValue( rPos.Row() ); }
    double      GetValue( USHORT nCol, USHORT nRow );
    void        GetFormula( USHORT nCol, USHORT nRow, String& rFormula,
                            BOOL bAsciiExport = FALSE );
    BOOL        GetNote( USHORT nCol, USHORT nRow, ScPostIt& rNote);

    CellType    GetCellType( const ScAddress& rPos ) const
                    { return aCol[rPos.Col()].GetCellType( rPos.Row() ); }
    CellType    GetCellType( USHORT nCol, USHORT nRow ) const;
    ScBaseCell* GetCell( const ScAddress& rPos ) const
                    { return aCol[rPos.Col()].GetCell( rPos.Row() ); }
    ScBaseCell* GetCell( USHORT nCol, USHORT nRow ) const;

    void        GetLastDataPos(USHORT& rCol, USHORT& rRow) const;

    BOOL        TestInsertRow( USHORT nStartCol, USHORT nEndCol, USHORT nSize );
    void        InsertRow( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nSize );
    void        DeleteRow( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nSize,
                            BOOL* pUndoOutline = NULL );

    BOOL        TestInsertCol( USHORT nStartRow, USHORT nEndRow, USHORT nSize );
    void        InsertCol( USHORT nStartCol, USHORT nStartRow, USHORT nEndRow, USHORT nSize );
    void        DeleteCol( USHORT nStartCol, USHORT nStartRow, USHORT nEndRow, USHORT nSize,
                            BOOL* pUndoOutline = NULL );

    void        DeleteArea(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nDelFlag);
    void        CopyToClip(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, ScTable* pTable,
                            BOOL bKeepScenarioFlags);
    void        CopyFromClip(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, short nDx, short nDy,
                                USHORT nInsFlag, BOOL bAsLink, ScTable* pTable);
    void        StartListeningInArea( USHORT nCol1, USHORT nRow1,
                                        USHORT nCol2, USHORT nRow2 );
    void        BroadcastInArea( USHORT nCol1, USHORT nRow1,
                                    USHORT nCol2, USHORT nRow2 );

    void        CopyToTable(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            USHORT nFlags, BOOL bMarked, ScTable* pDestTab,
                            const ScMarkData* pMarkData = NULL,
                            BOOL bAsLink = FALSE, BOOL bColRowFlags = TRUE);
    void        UndoToTable(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            USHORT nFlags, BOOL bMarked, ScTable* pDestTab,
                            const ScMarkData* pMarkData = NULL);

    void        TransposeClip( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                ScTable* pTransClip, USHORT nFlags, BOOL bAsLink );

                //  Markierung von diesem Dokument
    void        MixMarked( const ScMarkData& rMark, USHORT nFunction,
                            BOOL bSkipEmpty, ScTable* pSrcTab );
    void        MixData( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            USHORT nFunction, BOOL bSkipEmpty, ScTable* pSrcTab );

    void        CopyData( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                            USHORT nDestCol, USHORT nDestRow, USHORT nDestTab );

    void        CopyScenarioFrom( const ScTable* pSrcTab );
    void        CopyScenarioTo( ScTable* pDestTab ) const;
    BOOL        TestCopyScenarioTo( const ScTable* pDestTab ) const;
    void        MarkScenarioIn( ScMarkData& rMark, USHORT nNeededBits ) const;
    BOOL        HasScenarioRange( const ScRange& rRange ) const;
    void        InvalidateScenarioRanges();
    const ScRangeList* GetScenarioRanges() const;

    void        CopyUpdated( const ScTable* pPosTab, ScTable* pDestTab ) const;

    void        InvalidateTableArea()                       { bTableAreaValid = FALSE; }

    BOOL        GetCellArea( USHORT& rEndCol, USHORT& rEndRow ) const;          // FALSE = leer
    BOOL        GetTableArea( USHORT& rEndCol, USHORT& rEndRow ) const;
    BOOL        GetPrintArea( USHORT& rEndCol, USHORT& rEndRow, BOOL bNotes ) const;
    BOOL        GetPrintAreaHor( USHORT nStartRow, USHORT nEndRow,
                                USHORT& rEndCol, BOOL bNotes ) const;
    BOOL        GetPrintAreaVer( USHORT nStartCol, USHORT nEndCol,
                                USHORT& rEndRow, BOOL bNotes ) const;

    BOOL        GetDataStart( USHORT& rStartCol, USHORT& rStartRow ) const;

    void        ExtendPrintArea( OutputDevice* pDev,
                        USHORT nStartCol, USHORT nStartRow, USHORT& rEndCol, USHORT nEndRow );

    void        GetDataArea( USHORT& rStartCol, USHORT& rStartRow, USHORT& rEndCol, USHORT& rEndRow,
                                BOOL bIncludeOld );

    USHORT      GetEmptyLinesInBlock( USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow, ScDirection eDir );

    void        FindAreaPos( USHORT& rCol, USHORT& rRow, short nMovX, short nMovY );
    void        GetNextPos( USHORT& rCol, USHORT& rRow, short nMovX, short nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark );

    void        LimitChartArea( USHORT& rStartCol, USHORT& rStartRow, USHORT& rEndCol, USHORT& rEndRow );

    BOOL        HasData( USHORT nCol, USHORT nRow );
    BOOL        HasStringData( USHORT nCol, USHORT nRow );
    BOOL        HasValueData( USHORT nCol, USHORT nRow );
    USHORT      GetErrorData(USHORT nCol, USHORT nRow) const;
    BOOL        HasStringCells( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow ) const;

    USHORT      GetErrCode( const ScAddress& rPos ) const
                    { return aCol[rPos.Col()].GetErrCode( rPos.Row() ); }
    USHORT      GetErrCode( USHORT nCol, USHORT nRow ) const;

    void        ResetChanged( const ScRange& rRange );

    void        SetDirty();
    void        SetDirty( const ScRange& );
    void        SetDirtyAfterLoad();
    void        SetDirtyVar();
    void        SetTableOpDirty( const ScRange& );
    void        CalcAll();
    void        CalcAfterLoad();
    void        CompileAll();
    void        CompileXML();

    void        UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz,
                                    ScDocument* pUndoDoc = NULL, BOOL bIncludeDraw = TRUE );

    void        UpdateDrawRef( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz );

    void        UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc );

    void        UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY );

    void        UpdateInsertTab(USHORT nTable);
    void        UpdateInsertTabOnlyCells(USHORT nTable);
    void        UpdateDeleteTab( USHORT nTable, BOOL bIsMove, ScTable* pRefUndo = NULL );
    void        UpdateMoveTab(USHORT nOldPos, USHORT nNewPos, USHORT nTabNo, ScProgress& );
    void        UpdateCompile( BOOL bForceIfNameInUse = FALSE );
    void        SetTabNo(USHORT nNewTab);
    BOOL        IsRangeNameInUse(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                 USHORT nIndex) const;
    void        ReplaceRangeNamesInUse(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                      const ScIndexMap& rMap );
    void        Fill( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                        USHORT nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                        double nStepValue, double nMaxValue);
    String      GetAutoFillPreview( const ScRange& rSource, USHORT nEndX, USHORT nEndY );

    void        UpdateSelectionFunction( ScFunctionData& rData,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark );

    void        AutoFormat( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    USHORT nFormatNo );
    void        GetAutoFormatData(USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, ScAutoFormatData& rData);
    void        ScReplaceTabsStr( String& rStr, const String& rSrch, const String& rRepl ); // aus sw
    BOOL        SearchAndReplace(const SvxSearchItem& rSearchItem,
                                USHORT& rCol, USHORT& rRow, ScMarkData& rMark,
                                String& rUndoStr, ScDocument* pUndoDoc);

    void        FindMaxRotCol( RowInfo* pRowInfo, USHORT nArrCount, USHORT nX1, USHORT nX2 ) const;

    void        GetBorderLines( USHORT nCol, USHORT nRow,
                                const SvxBorderLine** ppLeft, const SvxBorderLine** ppTop,
                                const SvxBorderLine** ppRight, const SvxBorderLine** ppBottom ) const;

    BOOL        HasLines( const ScRange& rRange, Rectangle& rSizes ) const;
    BOOL        HasAttrib( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nMask ) const;
    BOOL        HasAttribSelection( const ScMarkData& rMark, USHORT nMask ) const;
    BOOL        ExtendMerge( USHORT nStartCol, USHORT nStartRow,
                                USHORT& rEndCol, USHORT& rEndRow,
                                BOOL bRefresh, BOOL bAttrs );
    const SfxPoolItem*      GetAttr( USHORT nCol, USHORT nRow, USHORT nWhich ) const;
    const ScPatternAttr*    GetPattern( USHORT nCol, USHORT nRow ) const;

    ULONG                   GetNumberFormat( const ScAddress& rPos ) const
                                { return aCol[rPos.Col()].GetNumberFormat( rPos.Row() ); }
    ULONG                   GetNumberFormat( USHORT nCol, USHORT nRow ) const;
    void                    MergeSelectionPattern( SfxItemSet** ppSet,
                                                const ScMarkData& rMark, BOOL bDeep ) const;
    void                    MergePatternArea( SfxItemSet** ppSet, USHORT nCol1, USHORT nRow1,
                                                USHORT nCol2, USHORT nRow2, BOOL bDeep ) const;
    void                    MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                                            ScLineFlags& rFlags,
                                            USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow ) const;
    void                    ApplyBlockFrame( const SvxBoxItem* pLineOuter,
                                            const SvxBoxInfoItem* pLineInner,
                                            USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow );

    void        ApplyAttr( USHORT nCol, USHORT nRow, const SfxPoolItem& rAttr );
    void        ApplyPattern( USHORT nCol, USHORT nRow, const ScPatternAttr& rAttr );
    void        ApplyPatternArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, const ScPatternAttr& rAttr );
    void        SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr, BOOL bPutToPool = FALSE )
                    { aCol[rPos.Col()].SetPattern( rPos.Row(), rAttr, bPutToPool ); }
    void        SetPattern( USHORT nCol, USHORT nRow, const ScPatternAttr& rAttr, BOOL bPutToPool = FALSE );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, short nNewType );

    void        ApplyStyle( USHORT nCol, USHORT nRow, const ScStyleSheet& rStyle );
    void        ApplyStyleArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, const ScStyleSheet& rStyle );
    void        ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);
    void        ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const SvxBorderLine* pLine, BOOL bColorOnly );

    const ScStyleSheet* GetStyle( USHORT nCol, USHORT nRow ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const;
    const ScStyleSheet* GetAreaStyle( BOOL& rFound, USHORT nCol1, USHORT nRow1,
                                                    USHORT nCol2, USHORT nRow2 ) const;

    void        StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY );

    BOOL        IsStyleSheetUsed( const SfxStyleSheetBase& rStyle ) const;

    BOOL        ApplyFlags( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, INT16 nFlags );
    BOOL        RemoveFlags( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, INT16 nFlags );

    void        ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark );
    void        DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark );

    void        ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark );
    void        ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark );

    const ScRange*  GetRepeatColRange() const   { return pRepeatColRange; }
    const ScRange*  GetRepeatRowRange() const   { return pRepeatRowRange; }
    void            SetRepeatColRange( const ScRange* pNew );
    void            SetRepeatRowRange( const ScRange* pNew );

    USHORT          GetPrintRangeCount() const          { return nPrintRangeCount; }
    const ScRange*  GetPrintRange(USHORT nPos) const;
    void            SetPrintRangeCount( USHORT nNew );
    void            SetPrintRange( USHORT nPos, const ScRange& rNew );
    void            FillPrintSaver( ScPrintSaverTab& rSaveTab ) const;
    void            RestorePrintRanges( const ScPrintSaverTab& rSaveTab );

    USHORT      GetOptimalColWidth( USHORT nCol, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bFormula, const ScMarkData* pMarkData,
                                    BOOL bSimpleTextImport );
    BOOL        SetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT nExtra,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bForce );
    long        GetNeededSize( USHORT nCol, USHORT nRow,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, BOOL bTotalSize );
    void        SetColWidth( USHORT nCol, USHORT nNewWidth );
    void        SetRowHeight( USHORT nRow, USHORT nNewHeight );
    BOOL        SetRowHeightRange( USHORT nStartRow, USHORT nEndRow, USHORT nNewHeight,
                                    double nPPTX, double nPPTY );
                        // nPPT fuer Test auf Veraenderung
    void        SetManualHeight( USHORT nStartRow, USHORT nEndRow, BOOL bManual );

    USHORT      GetColWidth( USHORT nCol ) const;
    USHORT      GetRowHeight( USHORT nRow ) const;
    ULONG       GetColOffset( USHORT nCol ) const;
    ULONG       GetRowOffset( USHORT nRow ) const;

    USHORT      GetOriginalWidth( USHORT nCol ) const;
    USHORT      GetOriginalHeight( USHORT nRow ) const;

    USHORT      GetHiddenRowCount( USHORT nRow ) const;

    void        ShowCol(USHORT nCol, BOOL bShow);
    void        ShowRow(USHORT nRow, BOOL bShow);
    void        DBShowRow(USHORT nRow, BOOL bShow);

    void        ShowRows(USHORT nRow1, USHORT nRow2, BOOL bShow);
    void        DBShowRows(USHORT nRow1, USHORT nRow2, BOOL bShow);

    void        SetColFlags( USHORT nCol, BYTE nNewFlags );
    void        SetRowFlags( USHORT nRow, BYTE nNewFlags );

    USHORT      GetLastFlaggedCol() const;
    USHORT      GetLastFlaggedRow() const;

    BOOL        IsFiltered(USHORT nRow) const;

    BYTE        GetColFlags( USHORT nCol ) const;
    BYTE        GetRowFlags( USHORT nRow ) const;

    BOOL        UpdateOutlineCol( USHORT nStartCol, USHORT nEndCol, BOOL bShow );
    BOOL        UpdateOutlineRow( USHORT nStartRow, USHORT nEndRow, BOOL bShow );

    void        UpdatePageBreaks( const ScRange* pUserArea );
    void        RemoveManualBreaks();
    BOOL        HasManualBreaks() const;

    void        StripHidden( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2 );
    void        ExtendHidden( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2 );

    BOOL        Load( SvStream& rStream, USHORT nVersion, ScProgress* pProgress );
    BOOL        Save( SvStream& rStream, long& rSavedDocCells, ScProgress* pProgress ) const;

    void        Sort(const ScSortParam& rSortParam, BOOL bKeepQuery);
    BOOL        ValidQuery(USHORT nRow, const ScQueryParam& rQueryParam, BOOL* pSpecial = NULL);
    void        TopTenQuery( ScQueryParam& );
    USHORT      Query(ScQueryParam& rQueryParam, BOOL bKeepSub);
    BOOL        CreateQueryParam(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, ScQueryParam& rQueryParam);

    void        GetFilterEntries(USHORT nCol, USHORT nRow1, USHORT nRow2, TypedStrCollection& rStrings);
    BOOL        GetDataEntries(USHORT nCol, USHORT nRow, TypedStrCollection& rStrings, BOOL bLimit);

    BOOL        HasColHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow );
    BOOL        HasRowHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow );

    void        DoColResize( USHORT nCol1, USHORT nCol2, USHORT nAdd );

    // maximale Stringlaengen einer Column, fuer z.B. dBase Export
    xub_StrLen  GetMaxStringLen( USHORT nCol,
                                    USHORT nRowStart, USHORT nRowEnd ) const;
    xub_StrLen  GetMaxNumberStringLen( USHORT& nPrecision,
                                    USHORT nCol,
                                    USHORT nRowStart, USHORT nRowEnd ) const;

    void        FindConditionalFormat( ULONG nKey, ScRangeList& rRanges );

    void        IncRecalcLevel()        { ++nRecalcLvl; }
    void        DecRecalcLevel()        { if (!--nRecalcLvl) SetDrawPageSize(); }

    BOOL        IsSortCollatorGlobal() const;
    void        InitSortCollator( const ScSortParam& rPar );
    void        DestroySortCollator();

private:
    void        FillSeries( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                USHORT nFillCount, FillDir eFillDir, FillCmd eFillCmd,
                                FillDateCmd eFillDateCmd,
                                double nStepValue, double nMaxValue, USHORT nMinDigits,
                                BOOL bAttribs, ScProgress& rProgress );
    void        FillAnalyse( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                FillCmd& rCmd, FillDateCmd& rDateCmd,
                                double& rInc, USHORT& rMinDigits,
                                ScUserListData*& rListData, USHORT& rListIndex);
    void        FillAuto( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                        USHORT nFillCount, FillDir eFillDir, ScProgress& rProgress );

    BOOL        ValidNextPos( USHORT nCol, USHORT nRow, const ScMarkData& rMark,
                                BOOL bMarked, BOOL bUnprotected );

    void        AutoFormatArea(USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                USHORT nIndex, USHORT nFormatNo);
    void        GetAutoFormatAttr(USHORT nCol, USHORT nRow, USHORT nIndex, ScAutoFormatData& rData);
    void        GetAutoFormatFrame(USHORT nCol, USHORT nRow, USHORT nFlags, USHORT nIndex, ScAutoFormatData& rData);
    BOOL        SearchCell(const SvxSearchItem& rSearchItem, USHORT nCol, USHORT nRow,
                            const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc);
    BOOL        Search(const SvxSearchItem& rSearchItem, USHORT& rCol, USHORT& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc);
    BOOL        SearchAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                        String& rUndoStr, ScDocument* pUndoDoc);
    BOOL        Replace(const SvxSearchItem& rSearchItem, USHORT& rCol, USHORT& rRow,
                        const ScMarkData& rMark, String& rUndoStr, ScDocument* pUndoDoc);
    BOOL        ReplaceAll(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                            String& rUndoStr, ScDocument* pUndoDoc);

    BOOL        SearchStyle(const SvxSearchItem& rSearchItem, USHORT& rCol, USHORT& rRow,
                                ScMarkData& rMark);
    BOOL        ReplaceStyle(const SvxSearchItem& rSearchItem, USHORT& rCol, USHORT& rRow,
                                ScMarkData& rMark, BOOL bIsUndo);
    BOOL        SearchAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark);
    BOOL        ReplaceAllStyle(const SvxSearchItem& rSearchItem, ScMarkData& rMark,
                                ScDocument* pUndoDoc);

                                // benutzen globalen SortParam:
    BOOL        IsSorted(USHORT nStart, USHORT nEnd);
    void        DecoladeRow( ScSortInfoArray*, USHORT nRow1, USHORT nRow2 );
    void        SwapCol(USHORT nCol1, USHORT nCol2);
    void        SwapRow(USHORT nRow1, USHORT nRow2);
    short       CompareCell( USHORT nSort,
                    ScBaseCell* pCell1, USHORT nCell1Col, USHORT nCell1Row,
                    ScBaseCell* pCell2, USHORT nCell2Col, USHORT nCell2Row );
    short       Compare(USHORT nIndex1, USHORT nIndex2);
    short       Compare( ScSortInfoArray*, USHORT nIndex1, USHORT nIndex2);
    ScSortInfoArray*    CreateSortInfoArray( USHORT nInd1, USHORT nInd2 );
    void        QuickSort( ScSortInfoArray*, short nLo, short nHi);
    void        SortReorder( ScSortInfoArray*, ScProgress& );

    BOOL        CreateExcelQuery(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, ScQueryParam& rQueryParam);
    BOOL        CreateStarQuery(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, ScQueryParam& rQueryParam);
    void        GetUpperCellString(USHORT nCol, USHORT nRow, String& rStr);

    BOOL        RefVisible(ScFormulaCell* pCell);

    BOOL        IsEmptyLine(USHORT nRow, USHORT nStartCol, USHORT nEndCol);

    void        IncDate(double& rVal, USHORT& nDayOfMonth, double nStep, FillDateCmd eCmd);
    void        FillFormula(USHORT& nFormulaCounter, BOOL bFirst, ScFormulaCell* pSrcCell,
                            USHORT nDestCol, USHORT nDestRow, BOOL bLast );
    void        UpdateInsertTabAbs(USHORT nNewPos);
    BOOL        GetNextSpellingCell(USHORT& rCol, USHORT& rRow, BOOL bInSel,
                                    const ScMarkData& rMark) const;
    BOOL        GetNextMarkedCell( USHORT& rCol, USHORT& rRow, const ScMarkData& rMark );
    void        SetDrawPageSize();
    BOOL        TestTabRefAbs(USHORT nTable);
    void        CompileDBFormula();
    void        CompileDBFormula( BOOL bCreateFormulaString );
    void        CompileNameFormula( BOOL bCreateFormulaString );
    void        CompileColRowNameFormula();

    void        StartListening( const ScAddress& rAddress, SfxListener* pListener );
    void        EndListening( const ScAddress& rAddress, SfxListener* pListener );
    void        StartAllListeners();
    void        StartRelNameListeners();
    void        SetRelNameDirty();

    USHORT      FillMaxRot( RowInfo* pRowInfo, USHORT nArrCount, USHORT nX1, USHORT nX2,
                            USHORT nCol, USHORT nAttrRow1, USHORT nAttrRow2, USHORT nArrY,
                            const ScPatternAttr* pPattern, const SfxItemSet* pCondSet ) const;

    // Idleberechnung der OutputDevice-Zelltextbreite
    void            InvalidateTextWidth( const ScAddress* pAdrFrom = NULL,
                                         const ScAddress* pAdrTo   = NULL,
                                         BOOL bBroadcast = FALSE );
};


#endif


