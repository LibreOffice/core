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

#ifndef SC_COLUMN_HXX
#define SC_COLUMN_HXX

#include "markarr.hxx"

#include "global.hxx"
class Fraction;
class OutputDevice;
class Rectangle;
namespace binfilter {

class SfxBroadcaster;
class SfxItemPoolCache;
class SfxItemSet;
class SfxListener;
class SfxPoolItem; 
class SfxStyleSheetBase;

class SvxBorderLine;
class SvxBoxInfoItem;
class SvxBoxItem;

class ScAttrIterator;
class ScAttrArray;
class ScBaseCell;
class ScDocument;
class ScFormulaCell;
class ScMarkData;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;
class ScPatternAttr;
class ScStyleSheet;
class ScBroadcasterList;
class TypedStrCollection;
class ScProgress;
struct ScFunctionData;
struct ScLineFlags;


#define COLUMN_DELTA	4


struct ScNeededSizeOptions
{
    const ScPatternAttr*	pPattern;
    BOOL					bFormula;
    BOOL					bSkipMerged;
    BOOL					bGetFont;
    BOOL					bTotalSize;

    ScNeededSizeOptions()
    {
        pPattern = NULL;
        bFormula = FALSE;
        bSkipMerged = TRUE;
        bGetFont = TRUE;
        bTotalSize = FALSE;
    }
};

struct ColEntry
{
    USHORT		nRow;
    ScBaseCell*	pCell;
};


class ScIndexMap;

class ScColumn
{
private:
    USHORT			nCol;
    USHORT			nTab;

    USHORT			nCount;
    USHORT			nLimit;
    ColEntry*		pItems;

    ScAttrArray*	pAttrArray;
    ScDocument*		pDocument;

friend class ScDocument;					// fuer FillInfo
friend class ScValueIterator;
friend class ScQueryValueIterator;
friend class ScColumnIterator;
friend class ScQueryCellIterator;
friend class ScMarkedDataIter;
friend class ScCellIterator;
friend class ScHorizontalCellIterator;

public:
static BOOL bDoubleAlloc;			// fuer Import: Groesse beim Allozieren verdoppeln

public:
                ScColumn();
                ~ScColumn();

    void		Init(USHORT nNewCol, USHORT nNewTab, ScDocument* pDoc);

    BOOL 		Search( USHORT nRow, USHORT& nIndex ) const;
    ScBaseCell*	GetCell( USHORT nRow ) const;
    void		Insert( USHORT nRow, ScBaseCell* pCell );
    void		Insert( USHORT nRow, ULONG nFormatIndex, ScBaseCell* pCell );
    void		Append( USHORT nRow, ScBaseCell* pCell );
    void		DeleteAtIndex( USHORT nIndex );
    void 	    FreeAll();
    void		Resize( USHORT nSize );

    BOOL		HasAttrib( USHORT nRow1, USHORT nRow2, USHORT nMask ) const;
    BOOL		ExtendMerge( USHORT nThisCol, USHORT nStartRow, USHORT nEndRow,
                                USHORT& rPaintCol, USHORT& rPaintRow,
                                BOOL bRefresh, BOOL bAttrs );

    BOOL		IsEmptyVisData(BOOL bNotes) const;		// ohne Broadcaster
    BOOL		IsEmptyData() const;
    BOOL		IsEmptyAttr() const;
/*N*/ 	BOOL		IsEmpty() const;

                // nur Daten:
    BOOL		HasDataAt(USHORT nRow) const;
    USHORT		GetLastVisDataPos(BOOL bNotes) const;				// ohne Broadcaster
    USHORT		GetFirstVisDataPos(BOOL bNotes) const;


    USHORT		GetBlockMatrixEdges( USHORT nRow1, USHORT nRow2, USHORT nMask ) const;
    BOOL		HasSelectionMatrixFragment(const ScMarkData& rMark) const;

                // Daten oder Attribute:

                // nur Attribute:
    BOOL		HasVisibleAttr( USHORT& rFirstRow, USHORT& rLastRow, BOOL bSkipFirst ) const;
    BOOL		IsVisibleAttrEqual( const ScColumn& rCol, USHORT nStartRow = 0,
                                    USHORT nEndRow = MAXROW ) const;
    BOOL		IsAllAttrEqual( const ScColumn& rCol, USHORT nStartRow, USHORT nEndRow ) const;

/*N*/ 	BOOL		TestInsertCol( USHORT nStartRow, USHORT nEndRow) const;
/*N*/ 	BOOL		TestInsertRow( USHORT nSize ) const;
/*N*/ 	void		InsertRow( USHORT nStartRow, USHORT nSize );
/*N*/ 	void		DeleteRow( USHORT nStartRow, USHORT nSize );
    void		DeleteRange( USHORT nStartIndex, USHORT nEndIndex, USHORT nDelFlag );
    void		DeleteArea(USHORT nStartRow, USHORT nEndRow, USHORT nDelFlag );


                //	Markierung von diesem Dokument


    ScAttrIterator* CreateAttrIterator( USHORT nStartRow, USHORT nEndRow ) const;

    USHORT		GetCol() const { return nCol; }

                //	UpdateSelectionFunction: Mehrfachselektion
    void		UpdateSelectionFunction( const ScMarkData& rMark,
                                    ScFunctionData& rData, const BYTE* pRowFlags,
                                    BOOL bDoExclude, USHORT nExStartRow, USHORT nExEndRow );
    void		UpdateAreaFunction( ScFunctionData& rData, BYTE* pRowFlags,
                                    USHORT nStartRow, USHORT nEndRow );

    void		CopyToColumn(USHORT nRow1, USHORT nRow2, USHORT nFlags, BOOL bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData = NULL,
                                BOOL bAsLink = FALSE );

    void		MarkScenarioIn( ScMarkData& rDestMark ) const;


/*N*/ 	void		SwapCol(ScColumn& rCol);
/*N*/ 	void		MoveTo(USHORT nStartRow, USHORT nEndRow, ScColumn& rCol);

    BOOL		HasEditCells(USHORT nStartRow, USHORT nEndRow, USHORT& rFirst) const;

                //	TRUE = Zahlformat gesetzt
    BOOL		SetString( USHORT nRow, USHORT nTab, const String& rString );
    void		SetValue( USHORT nRow, const double& rVal);
    void		SetNote( USHORT nRow, const ScPostIt& rNote );
    void		SetError( USHORT nRow, const USHORT nError);

    void		GetString( USHORT nRow, String& rString ) const;
    void		GetInputString( USHORT nRow, String& rString ) const;
    double		GetValue( USHORT nRow ) const;
    BOOL		GetNote( USHORT nRow, ScPostIt& rNote ) const;
    CellType	GetCellType( USHORT nRow ) const;
    USHORT		GetCellCount() const { return nCount; }
    long		GetWeightedCount() const;
    USHORT		GetErrCode( USHORT nRow ) const;

    BOOL		HasStringData( USHORT nRow ) const;
    BOOL		HasValueData( USHORT nRow ) const;

    void		SetDirty();
    void		SetDirtyVar();
    void		SetDirtyAfterLoad();
     void		SetTableOpDirty( const ScRange& );
    void		CalcAll();
    void		CalcAfterLoad();
    void		CompileAll();
    void		CompileXML( ScProgress& rProgress );

    void		ResetChanged( USHORT nStartRow, USHORT nEndRow );

    void		UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                     USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                     short nDx, short nDy, short nDz,
                                     ScDocument* pUndoDoc = NULL );
    void		UpdateInsertTab( USHORT nTable);
    void		UpdateInsertTabOnlyCells( USHORT nTable);
     void		UpdateDeleteTab( USHORT nTable, BOOL bIsMove, ScColumn* pRefUndo = NULL );
    void		UpdateCompile( BOOL bForceIfNameInUse = FALSE );

    void		SetTabNo(USHORT nNewTab);
     BOOL		IsRangeNameInUse(USHORT nRow1, USHORT nRow2, USHORT nIndex) const;

    const SfxPoolItem*		GetAttr( USHORT nRow, USHORT nWhich ) const;
    const ScPatternAttr*	GetPattern( USHORT nRow ) const;
    ULONG		GetNumberFormat( USHORT nRow ) const;

    void		MergeSelectionPattern( SfxItemSet** ppSet, const ScMarkData& rMark, BOOL bDeep ) const;
    void		MergePatternArea( SfxItemSet** ppSet, USHORT nRow1, USHORT nRow2, BOOL bDeep ) const;

    void		ApplyAttr( USHORT nRow, const SfxPoolItem& rAttr );
    void		ApplyPatternArea( USHORT nStartRow, USHORT nEndRow, const ScPatternAttr& rPatAttr );

    void		ApplyStyleArea( USHORT nStartRow, USHORT nEndRow, const ScStyleSheet& rStyle );
    void 		ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);

    const ScStyleSheet*	GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const;
    const ScStyleSheet*	GetAreaStyle( BOOL& rFound, USHORT nRow1, USHORT nRow2 ) const;

    void		FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, BOOL* pUsed, BOOL bReset );


    BOOL		ApplyFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags );
/*N*/ 	BOOL		RemoveFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags );
/*N*/ 	void		ClearItems( USHORT nStartRow, USHORT nEndRow, const USHORT* pWhich );


    short		ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark );


    long		GetNeededSize( USHORT nRow, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, const ScNeededSizeOptions& rOptions );
    USHORT		GetOptimalColWidth( OutputDevice* pDev, double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bFormula, USHORT nOldWidth,
                                    const ScMarkData* pMarkData,
                                    BOOL bSimpleTextImport );
    void		GetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT* pHeight,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bShrink, USHORT nMinHeight, USHORT nMinStart );
private:
    long		GetSimpleTextNeededSize( USHORT nIndex, OutputDevice* pDev,
                                    BOOL bWidth );
public:



    USHORT		NoteCount( USHORT nMaxRow = MAXROW ) const;

    void		LoadData( SvStream& rStream );
    void		SaveData( SvStream& rStream ) const;

    void		LoadNotes( SvStream& rStream );
    void		SaveNotes( SvStream& rStream ) const;

    BOOL		Load( SvStream& rStream, ScMultipleReadHeader& rHdr );
    BOOL		Save( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;
    BOOL		TestTabRefAbs(USHORT nTable);


    void		StartListening( SfxListener& rLst, USHORT nRow );
    void		EndListening( SfxListener& rLst, USHORT nRow );
/*N*/ 	void		MoveListeners( ScBroadcasterList& rSource, USHORT nDestRow );
    void		StartAllListeners();
/*N*/    void        StartNameListeners( BOOL bOnlyRelNames );
/*N*/ 	void		SetRelNameDirty();

    void 		CompileDBFormula( BOOL bCreateFormulaString );
    void 		CompileNameFormula( BOOL bCreateFormulaString );
    void 		CompileColRowNameFormula();

    // maximale Stringlaengen einer Column, fuer z.B. dBase Export

private:
    ScBaseCell* CloneCell(USHORT nIndex, USHORT nFlags,
                            ScDocument* pDestDoc, const ScAddress& rDestPos);
    void		CorrectSymbolCells( CharSet eStreamCharSet );
};


class ScColumnIterator					// alle Daten eines Bereichs durchgehen
{
    const ScColumn*		pColumn;
    USHORT				nPos;
    USHORT				nTop;
    USHORT				nBottom;
public:
                ScColumnIterator( const ScColumn* pCol, USHORT nStart=0, USHORT nEnd=MAXROW );
                ~ScColumnIterator();

    BOOL		Next( USHORT& rRow, ScBaseCell*& rpCell );
};


class ScMarkedDataIter					// Daten in selektierten Bereichen durchgehen
{
    const ScColumn*		pColumn;
    USHORT				nPos;
    ScMarkArrayIter*	pMarkIter;
    USHORT				nTop;
    USHORT				nBottom;
    BOOL				bNext;
    BOOL				bAll;

public:
                ScMarkedDataIter( const ScColumn* pCol, const ScMarkData* pMarkData,
                                    BOOL bAllIfNone = FALSE );
                ~ScMarkedDataIter();

    BOOL		Next( USHORT& rIndex );
};


} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
