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

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
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
class CollatorWrapper;
namespace binfilter {
class SfxItemSet;
class SfxStyleSheetBase;

class SvxBoxInfoItem;
class SvxBoxItem;

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


class ScTable
{
private:
                                            //	Daten pro Tabelle	------------------
    ScColumn		aCol[MAXCOL+1];

    String			aName;
    String			aComment;
    BOOL			bScenario;

    String			aLinkDoc;
    String			aLinkFlt;
    String			aLinkOpt;
    String			aLinkTab;
    ULONG			nLinkRefreshDelay;
    BYTE			nLinkMode;

    // Seitenformatvorlage
    String			aPageStyle;
    BOOL			bPageSizeValid;
    Size			aPageSizeTwips;					// Groesse der Druck-Seite
    USHORT			nRepeatStartX;					// Wiederholungszeilen/Spalten
    USHORT			nRepeatEndX;					// REPEAT_NONE, wenn nicht benutzt
    USHORT			nRepeatStartY;
    USHORT			nRepeatEndY;

    BOOL			bProtected;
    ::com::sun::star::uno::Sequence<sal_Int8>	aProtectPass;

    USHORT*			pColWidth;
    USHORT*			pRowHeight;

    BYTE*			pColFlags;
    BYTE*			pRowFlags;

    ScOutlineTable*	pOutlineTable;

    USHORT			nTableAreaX;
    USHORT			nTableAreaY;
    BOOL			bTableAreaValid;

                                            //	interne Verwaltung	------------------
    BOOL			bVisible;

    USHORT			nTab;
    USHORT			nRecalcLvl;				// Rekursionslevel Size-Recalc
    ScDocument*		pDocument;
    ::utl::SearchParam*	pSearchParam;
    ::utl::TextSearch*	pSearchText;

    // SortierParameter um den Stackbedarf von Quicksort zu Minimieren
    ScSortParam		aSortParam;
    CollatorWrapper*	pSortCollator;
    BOOL			bGlobalKeepQuery;
    BOOL			bSharedNameInserted;

    USHORT			nPrintRangeCount;
    ScRange*		pPrintRanges;
    ScRange*		pRepeatColRange;
    ScRange*		pRepeatRowRange;

    USHORT			nLockCount;

    ScRangeList*	pScenarioRanges;
    Color			aScenarioColor;
    USHORT			nScenarioFlags;
    BOOL			bActiveScenario;

friend class ScDocument;					// fuer FillInfo
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

    ScOutlineTable*	GetOutlineTable()				{ return pOutlineTable; }

    long		GetCellCount() const;
    long		GetWeightedCount() const;

    BOOL		SetOutlineTable( const ScOutlineTable* pNewOutline );
    void		StartOutlineTable();



    BOOL		IsVisible() const							 { return bVisible; }
    void		SetVisible( BOOL bVis );

    BOOL		IsScenario() const							 { return bScenario; }
    void		SetScenario( BOOL bFlag );
    void 		GetScenarioComment( String& rComment) const	 { rComment = aComment; }
    void		SetScenarioComment( const String& rComment ) { aComment = rComment; }
    const Color& GetScenarioColor() const					 { return aScenarioColor; }
    void		SetScenarioColor(const Color& rNew)			 { aScenarioColor = rNew; }
    USHORT		GetScenarioFlags() const					 { return nScenarioFlags; }
    void		SetScenarioFlags(USHORT nNew)				 { nScenarioFlags = nNew; }
    void		SetActiveScenario(BOOL bSet)				 { bActiveScenario = bSet; }
    BOOL		IsActiveScenario() const					 { return bActiveScenario; }

    BYTE		GetLinkMode() const							{ return nLinkMode; }
    BOOL		IsLinked() const							{ return nLinkMode != SC_LINK_NONE; }
    const String& GetLinkDoc() const						{ return aLinkDoc; }
    const String& GetLinkFlt() const						{ return aLinkFlt; }
    const String& GetLinkOpt() const						{ return aLinkOpt; }
    const String& GetLinkTab() const						{ return aLinkTab; }
    ULONG		GetLinkRefreshDelay() const					{ return nLinkRefreshDelay; }

    void		SetLink( BYTE nMode, const String& rDoc, const String& rFlt,
                        const String& rOpt, const String& rTab, ULONG nRefreshDelay );

    void		GetName( String& rName ) const;
    void		SetName( const String& rNewName );

    const String&	GetPageStyle() const					{ return aPageStyle; }
    void			SetPageStyle( const String& rName );

    BOOL			IsProtected() const						{ return bProtected; }
    const ::com::sun::star::uno::Sequence<sal_Int8>&	GetPassword() const						{ return aProtectPass; }
    void			SetProtection( BOOL bProtect, const ::com::sun::star::uno::Sequence<sal_Int8>& rPasswd )
                                        { bProtected = bProtect; aProtectPass = rPasswd; }

    Size			GetPageSize() const;
    void			SetPageSize( const Size& rSize );
    void			SetRepeatArea( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nEndRow );


    void		LockTable();
    void		UnlockTable();

    BOOL		IsBlockEditable( USHORT nCol1, USHORT nRow1, USHORT nCol2,
                        USHORT nRow2, BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL		IsSelectionEditable( const ScMarkData& rMark,
                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;

    BOOL		HasBlockMatrixFragment( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) const;
    BOOL		HasSelectionMatrixFragment( const ScMarkData& rMark ) const;


    void		PutCell( const ScAddress&, ScBaseCell* pCell );
    void		PutCell( USHORT nCol, USHORT nRow, ScBaseCell* pCell );
    void		PutCell(USHORT nCol, USHORT nRow, ULONG nFormatIndex, ScBaseCell* pCell);
                //	TRUE = Zahlformat gesetzt
    BOOL		SetString( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString );
    void		SetValue( USHORT nCol, USHORT nRow, const double& rVal );
    void		SetNote( USHORT nCol, USHORT nRow, const ScPostIt& rNote);
    void 		SetError( USHORT nCol, USHORT nRow, USHORT nError);

    void		GetString( USHORT nCol, USHORT nRow, String& rString );
    void		GetInputString( USHORT nCol, USHORT nRow, String& rString );
    double		GetValue( const ScAddress& rPos ) const
                    { return aCol[rPos.Col()].GetValue( rPos.Row() ); }
    double		GetValue( USHORT nCol, USHORT nRow );
    BOOL		GetNote( USHORT nCol, USHORT nRow, ScPostIt& rNote);

    CellType	GetCellType( const ScAddress& rPos ) const
                    { return aCol[rPos.Col()].GetCellType( rPos.Row() ); }
    CellType	GetCellType( USHORT nCol, USHORT nRow ) const;
    ScBaseCell*	GetCell( const ScAddress& rPos ) const
                    { return aCol[rPos.Col()].GetCell( rPos.Row() ); }
    ScBaseCell*	GetCell( USHORT nCol, USHORT nRow ) const;


/*N*/ 	BOOL		TestInsertRow( USHORT nStartCol, USHORT nEndCol, USHORT nSize );
/*N*/ 	void		InsertRow( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nSize );
/*N*/ 	void		DeleteRow( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nSize,
/*N*/ 							BOOL* pUndoOutline = NULL );
/*N*/
/*N*/ 	BOOL		TestInsertCol( USHORT nStartRow, USHORT nEndRow, USHORT nSize );
/*N*/ 	void		InsertCol( USHORT nStartCol, USHORT nStartRow, USHORT nEndRow, USHORT nSize );
/*N*/ 	void		DeleteCol( USHORT nStartCol, USHORT nStartRow, USHORT nEndRow, USHORT nSize,
/*N*/ 							BOOL* pUndoOutline = NULL );

    void		DeleteArea(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nDelFlag);

    void		CopyToTable(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            USHORT nFlags, BOOL bMarked, ScTable* pDestTab,
                            const ScMarkData* pMarkData = NULL,
                            BOOL bAsLink = FALSE, BOOL bColRowFlags = TRUE);


                //	Markierung von diesem Dokument


    void		MarkScenarioIn( ScMarkData& rMark, USHORT nNeededBits ) const;
    void		InvalidateScenarioRanges();
    const ScRangeList* GetScenarioRanges() const;


    void		InvalidateTableArea()						{ bTableAreaValid = FALSE; }

    BOOL		GetCellArea( USHORT& rEndCol, USHORT& rEndRow ) const;			// FALSE = leer
    BOOL		GetTableArea( USHORT& rEndCol, USHORT& rEndRow ) const;
    BOOL		GetPrintArea( USHORT& rEndCol, USHORT& rEndRow, BOOL bNotes ) const;

    BOOL		GetDataStart( USHORT& rStartCol, USHORT& rStartRow ) const;

    void		ExtendPrintArea( OutputDevice* pDev,
                        USHORT nStartCol, USHORT nStartRow, USHORT& rEndCol, USHORT nEndRow );

    void		GetDataArea( USHORT& rStartCol, USHORT& rStartRow, USHORT& rEndCol, USHORT& rEndRow,
                                BOOL bIncludeOld );


    void		GetNextPos( USHORT& rCol, USHORT& rRow, short nMovX, short nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark );


    BOOL		HasData( USHORT nCol, USHORT nRow );
    BOOL		HasStringData( USHORT nCol, USHORT nRow );
    BOOL		HasValueData( USHORT nCol, USHORT nRow );

    USHORT		GetErrCode( const ScAddress& rPos ) const
                    { return aCol[rPos.Col()].GetErrCode( rPos.Row() ); }

    void		ResetChanged( const ScRange& rRange );

    void		SetDirty();
    void		SetDirty( const ScRange& );
    void		SetDirtyAfterLoad();
    void		SetDirtyVar();
    void		SetTableOpDirty( const ScRange& );
    void		CalcAll();
    void		CalcAfterLoad();
    void		CompileAll();
    void		CompileXML( ScProgress& rProgress );

    void		UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz,
                                    ScDocument* pUndoDoc = NULL, BOOL bIncludeDraw = TRUE );

    void		UpdateDrawRef( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz );



    void		UpdateInsertTab(USHORT nTable);
    void 		UpdateDeleteTab( USHORT nTable, BOOL bIsMove, ScTable* pRefUndo = NULL );
    void		UpdateCompile( BOOL bForceIfNameInUse = FALSE );
    void		SetTabNo(USHORT nNewTab);
    BOOL		IsRangeNameInUse(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                  USHORT nIndex) const;
    void 		ReplaceRangeNamesInUse(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                      const ScIndexMap& rMap );

    void		UpdateSelectionFunction( ScFunctionData& rData,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark );

    void		GetBorderLines( USHORT nCol, USHORT nRow,
                                const SvxBorderLine** ppLeft, const SvxBorderLine** ppTop,
                                const SvxBorderLine** ppRight, const SvxBorderLine** ppBottom ) const;

    BOOL		HasAttrib( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nMask ) const;
    BOOL		ExtendMerge( USHORT nStartCol, USHORT nStartRow,
                                USHORT& rEndCol, USHORT& rEndRow,
                                BOOL bRefresh, BOOL bAttrs );
    const SfxPoolItem*		GetAttr( USHORT nCol, USHORT nRow, USHORT nWhich ) const;
    const ScPatternAttr*	GetPattern( USHORT nCol, USHORT nRow ) const;

    ULONG					GetNumberFormat( const ScAddress& rPos ) const
                                { return aCol[rPos.Col()].GetNumberFormat( rPos.Row() ); }
    ULONG					GetNumberFormat( USHORT nCol, USHORT nRow ) const;
    void					MergeSelectionPattern( SfxItemSet** ppSet,
                                                const ScMarkData& rMark, BOOL bDeep ) const;
    void					MergePatternArea( SfxItemSet** ppSet, USHORT nCol1, USHORT nRow1,
                                                USHORT nCol2, USHORT nRow2, BOOL bDeep ) const;
    void					MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                                            ScLineFlags& rFlags,
                                            USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow ) const;
    void					ApplyBlockFrame( const SvxBoxItem* pLineOuter,
                                            const SvxBoxInfoItem* pLineInner,
                                            USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow );

    void		ApplyAttr( USHORT nCol, USHORT nRow, const SfxPoolItem& rAttr );
    void		ApplyPatternArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, const ScPatternAttr& rAttr );
    void		ApplyStyleArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, const ScStyleSheet& rStyle );
    void		ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);

    const ScStyleSheet*	GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const;
    const ScStyleSheet*	GetAreaStyle( BOOL& rFound, USHORT nCol1, USHORT nRow1,
                                                    USHORT nCol2, USHORT nRow2 ) const;

    void		StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY );

    BOOL		ApplyFlags( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, INT16 nFlags );

    void		ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark );


    const ScRange*	GetRepeatColRange() const	{ return pRepeatColRange; }
    const ScRange*	GetRepeatRowRange() const	{ return pRepeatRowRange; }
    void			SetRepeatColRange( const ScRange* pNew );
    void			SetRepeatRowRange( const ScRange* pNew );

    USHORT			GetPrintRangeCount() const			{ return nPrintRangeCount; }
    const ScRange*	GetPrintRange(USHORT nPos) const;
    void			SetPrintRangeCount( USHORT nNew );
    void			SetPrintRange( USHORT nPos, const ScRange& rNew );
    void			FillPrintSaver( ScPrintSaverTab& rSaveTab ) const;

    USHORT		GetOptimalColWidth( USHORT nCol, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bFormula, const ScMarkData* pMarkData,
                                    BOOL bSimpleTextImport );
    BOOL		SetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT nExtra,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bForce );
    void		SetColWidth( USHORT nCol, USHORT nNewWidth );
    void		SetRowHeight( USHORT nRow, USHORT nNewHeight );
    BOOL		SetRowHeightRange( USHORT nStartRow, USHORT nEndRow, USHORT nNewHeight,
                                    double nPPTX, double nPPTY );
                        // nPPT fuer Test auf Veraenderung
    void		SetManualHeight( USHORT nStartRow, USHORT nEndRow, BOOL bManual );

    USHORT		GetColWidth( USHORT nCol ) const;
    USHORT		GetRowHeight( USHORT nRow ) const;
    ULONG		GetColOffset( USHORT nCol ) const;
    ULONG		GetRowOffset( USHORT nRow ) const;

    USHORT		GetOriginalWidth( USHORT nCol ) const;
    USHORT		GetOriginalHeight( USHORT nRow ) const;


    USHORT		GetHiddenRowCount( USHORT nRow ) const;

    void		ShowCol(USHORT nCol, BOOL bShow);

    void		ShowRows(USHORT nRow1, USHORT nRow2, BOOL bShow);

    void		SetRowFlags( USHORT nRow, BYTE nNewFlags );

                /// @return  the index of the last column with any set flags (auto-pagebreak is ignored).
                /// @return  the index of the last row with any set flags (auto-pagebreak is ignored).

                /// @return  the index of the last changed column (flags and column width, auto pagebreak is ignored).
    USHORT      GetLastChangedCol() const;
                /// @return  the index of the last changed row (flags and row height, auto pagebreak is ignored).
    USHORT      GetLastChangedRow() const;

    BOOL		IsFiltered(USHORT nRow) const;

    BYTE		GetColFlags( USHORT nCol ) const;
    BYTE		GetRowFlags( USHORT nRow ) const;

    BOOL		UpdateOutlineCol( USHORT nStartCol, USHORT nEndCol, BOOL bShow );
    BOOL		UpdateOutlineRow( USHORT nStartRow, USHORT nEndRow, BOOL bShow );

    void		UpdatePageBreaks( const ScRange* pUserArea );
    void		RemoveManualBreaks();

    BOOL		Load( SvStream& rStream, USHORT nVersion, ScProgress* pProgress );
    BOOL		Save( SvStream& rStream, long& rSavedDocCells, ScProgress* pProgress ) const;

    BOOL        ValidQuery(USHORT nRow, const ScQueryParam& rQueryParam,
                    BOOL* pSpecial = NULL, ScBaseCell* pCell = NULL,
                    BOOL* pbTestEqualCondition = NULL );
    BOOL		CreateQueryParam(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, ScQueryParam& rQueryParam);



    void		DoColResize( USHORT nCol1, USHORT nCol2, USHORT nAdd );

    // maximale Stringlaengen einer Column, fuer z.B. dBase Export

    void		FindConditionalFormat( ULONG nKey, ScRangeList& rRanges );

    void		IncRecalcLevel()		{ ++nRecalcLvl; }
    void		DecRecalcLevel()		{ if (!--nRecalcLvl) SetDrawPageSize(); }

    void		DestroySortCollator();

private:

                                // benutzen globalen SortParam:




    BOOL		GetNextMarkedCell( USHORT& rCol, USHORT& rRow, const ScMarkData& rMark );
    void		SetDrawPageSize();
    BOOL		TestTabRefAbs(USHORT nTable);
    void 		CompileDBFormula( BOOL bCreateFormulaString );
    void 		CompileNameFormula( BOOL bCreateFormulaString );
    void 		CompileColRowNameFormula();

    void		StartListening( const ScAddress& rAddress, SfxListener* pListener );
    void		EndListening( const ScAddress& rAddress, SfxListener* pListener );
    void		StartAllListeners();
/*N*/    void        StartNameListeners( BOOL bOnlyRelNames );
/*N*/ 	void		SetRelNameDirty();

    // Idleberechnung der OutputDevice-Zelltextbreite
    void			InvalidateTextWidth( const ScAddress* pAdrFrom = NULL,
                                         const ScAddress* pAdrTo   = NULL,
                                         BOOL bBroadcast = FALSE );
};


} //namespace binfilter
#endif


