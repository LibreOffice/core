/*************************************************************************
 *
 *  $RCSfile: column.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:48 $
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

#ifndef SC_COLUMN_HXX
#define SC_COLUMN_HXX

#ifndef SC_MARKARR_HXX
#include "markarr.hxx"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class Fraction;
class OutputDevice;
class Rectangle;
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
struct ScFunctionData;
struct ScLineFlags;


#define COLUMN_DELTA    4


struct ScNeededSizeOptions
{
    const ScPatternAttr*    pPattern;
    BOOL                    bFormula;
    BOOL                    bSkipMerged;
    BOOL                    bGetFont;
    BOOL                    bTotalSize;

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
    USHORT      nRow;
    ScBaseCell* pCell;
};


class ScIndexMap;

class ScColumn
{
private:
    USHORT          nCol;
    USHORT          nTab;

    USHORT          nCount;
    USHORT          nLimit;
    ColEntry*       pItems;

    ScAttrArray*    pAttrArray;
    ScDocument*     pDocument;

friend class ScDocument;                    // fuer FillInfo
friend class ScDocumentIterator;
friend class ScValueIterator;
friend class ScQueryValueIterator;
friend class ScColumnIterator;
friend class ScQueryCellIterator;
friend class ScMarkedDataIter;
friend class ScCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;

public:
static BOOL bDoubleAlloc;           // fuer Import: Groesse beim Allozieren verdoppeln

public:
                ScColumn();
                ~ScColumn();

    void        Init(USHORT nNewCol, USHORT nNewTab, ScDocument* pDoc);

    BOOL        Search( USHORT nRow, USHORT& nIndex ) const;
    ScBaseCell* GetCell( USHORT nRow ) const;
    void        Insert( USHORT nRow, ScBaseCell* pCell );
    void        Insert( USHORT nRow, ULONG nFormatIndex, ScBaseCell* pCell );
    void        Append( USHORT nRow, ScBaseCell* pCell );
    void        Delete( USHORT nRow );
    void        DeleteAtIndex( USHORT nIndex );
    void        FreeAll();
    void        Resize( USHORT nSize );
    void        SetNewRow( USHORT nRow1, USHORT nRow2 );
    void        SwapRow( USHORT nRow1, USHORT nRow2 );
    void        SwapCell( USHORT nRow, ScColumn& rCol);

    BOOL        HasLines( USHORT nRow1, USHORT nRow2, Rectangle& rSizes,
                            BOOL bLeft, BOOL bRight ) const;
    BOOL        HasAttrib( USHORT nRow1, USHORT nRow2, USHORT nMask ) const;
    BOOL        HasAttribSelection( const ScMarkData& rMark, USHORT nMask ) const;
    BOOL        ExtendMerge( USHORT nThisCol, USHORT nStartRow, USHORT nEndRow,
                                USHORT& rPaintCol, USHORT& rPaintRow,
                                BOOL bRefresh, BOOL bAttrs );

    BOOL        IsEmptyVisData(BOOL bNotes) const;      // ohne Broadcaster
    BOOL        IsEmptyData() const;
    BOOL        IsEmptyAttr() const;
    BOOL        IsEmpty() const;

                // nur Daten:
    BOOL        IsEmptyBlock(USHORT nStartRow, USHORT nEndRow) const;
    USHORT      GetEmptyLinesInBlock( USHORT nStartRow, USHORT nEndRow, ScDirection eDir ) const;
    BOOL        HasDataAt(USHORT nRow) const;
    BOOL        HasVisibleDataAt(USHORT nRow) const;
    USHORT      GetFirstDataPos() const;
    USHORT      GetLastDataPos() const;
    USHORT      GetLastVisDataPos(BOOL bNotes) const;               // ohne Broadcaster
    USHORT      GetFirstVisDataPos(BOOL bNotes) const;
    BOOL        GetPrevDataPos(USHORT& rRow) const;
    BOOL        GetNextDataPos(USHORT& rRow) const;
    void        FindDataAreaPos(USHORT& rRow, short nMovY) const;   // (ohne Broadcaster)
    void        FindUsed( USHORT nStartRow, USHORT nEndRow, BOOL* pUsed ) const;

    USHORT      VisibleCount( USHORT nStartRow, USHORT nEndRow ) const;

    USHORT      GetBlockMatrixEdges( USHORT nRow1, USHORT nRow2, USHORT nMask ) const;
    BOOL        HasSelectionMatrixFragment(const ScMarkData& rMark) const;

                // Daten oder Attribute:
    USHORT      GetFirstEntryPos() const;
    USHORT      GetLastEntryPos() const;

                // nur Attribute:
    USHORT      GetLastAttrPos() const;
    BOOL        HasVisibleAttr( USHORT& rFirstRow, USHORT& rLastRow, BOOL bSkipFirst ) const;
    BOOL        HasVisibleAttrIn( USHORT nStartRow, USHORT nEndRow ) const;
    BOOL        IsVisibleAttrEqual( const ScColumn& rCol, USHORT nStartRow = 0,
                                    USHORT nEndRow = MAXROW ) const;
    BOOL        IsAllAttrEqual( const ScColumn& rCol, USHORT nStartRow, USHORT nEndRow ) const;

    BOOL        TestInsertCol( USHORT nStartRow, USHORT nEndRow) const;
    BOOL        TestInsertRow( USHORT nSize ) const;
    void        InsertRow( USHORT nStartRow, USHORT nSize );
    void        DeleteRow( USHORT nStartRow, USHORT nSize );
    void        DeleteRange( USHORT nStartIndex, USHORT nEndIndex, USHORT nDelFlag );
    void        DeleteArea(USHORT nStartRow, USHORT nEndRow, USHORT nDelFlag );
    void        CopyToClip(USHORT nRow1, USHORT nRow2, ScColumn& rColumn, BOOL bKeepScenarioFlags);
    void        CopyFromClip(USHORT nRow1, USHORT nRow2, short nDy,
                                USHORT nInsFlag, BOOL bAsLink, ScColumn& rColumn);
    void        StartListeningInArea( USHORT nRow1, USHORT nRow2 );
    void        BroadcastInArea( USHORT nRow1, USHORT nRow2 );

    void        RemoveEditAttribs( USHORT nStartRow, USHORT nEndRow );

                //  Markierung von diesem Dokument
    void        MixMarked( const ScMarkData& rMark, USHORT nFunction,
                            BOOL bSkipEmpty, ScColumn& rSrcCol );
    void        MixData( USHORT nRow1, USHORT nRow2, USHORT nFunction, BOOL bSkipEmpty,
                            ScColumn& rSrcCol );

    ScFormulaCell*  CreateRefCell( ScDocument* pDestDoc, const ScAddress& rDestPos,
                                    USHORT nIndex, USHORT nFlags ) const;

    ScAttrIterator* CreateAttrIterator( USHORT nStartRow, USHORT nEndRow ) const;

    USHORT      GetCol() const { return nCol; }

                //  UpdateSelectionFunction: Mehrfachselektion
    void        UpdateSelectionFunction( const ScMarkData& rMark,
                                    ScFunctionData& rData, const BYTE* pRowFlags,
                                    BOOL bDoExclude, USHORT nExStartRow, USHORT nExEndRow );
    void        UpdateAreaFunction( ScFunctionData& rData, BYTE* pRowFlags,
                                    USHORT nStartRow, USHORT nEndRow );

    void        CopyToColumn(USHORT nRow1, USHORT nRow2, USHORT nFlags, BOOL bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData = NULL,
                                BOOL bAsLink = FALSE );
    void        UndoToColumn(USHORT nRow1, USHORT nRow2, USHORT nFlags, BOOL bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData = NULL );

    void        CopyScenarioFrom( const ScColumn& rSrcCol );
    void        CopyScenarioTo( ScColumn& rDestCol ) const;
    BOOL        TestCopyScenarioTo( const ScColumn& rDestCol ) const;
    void        MarkScenarioIn( ScMarkData& rDestMark ) const;

    void        CopyUpdated( const ScColumn& rPosCol, ScColumn& rDestCol ) const;

    void        SwapCol(ScColumn& rCol);
    void        MoveTo(USHORT nStartRow, USHORT nEndRow, ScColumn& rCol);

    BOOL        HasEditCells(USHORT nStartRow, USHORT nEndRow, USHORT& rFirst) const;

                //  TRUE = Zahlformat gesetzt
    BOOL        SetString( USHORT nRow, USHORT nTab, const String& rString );
    void        SetValue( USHORT nRow, const double& rVal);
    void        SetNote( USHORT nRow, const ScPostIt& rNote );
    void        SetError( USHORT nRow, const USHORT nError);

    void        GetString( USHORT nRow, String& rString ) const;
    void        GetInputString( USHORT nRow, String& rString ) const;
    double      GetValue( USHORT nRow ) const;
    void        GetFormula( USHORT nRow, String& rFormula,
                            BOOL bAsciiExport = FALSE ) const;
    BOOL        GetNote( USHORT nRow, ScPostIt& rNote ) const;
    CellType    GetCellType( USHORT nRow ) const;
    USHORT      GetCellCount() const { return nCount; }
    long        GetWeightedCount() const;
    ULONG       GetCodeCount() const;       // RPN-Code in Formeln
    USHORT      GetErrCode( USHORT nRow ) const;

    BOOL        HasStringData( USHORT nRow ) const;
    BOOL        HasValueData( USHORT nRow ) const;
    USHORT      GetErrorData(USHORT nCol) const;
    BOOL        HasStringCells( USHORT nStartRow, USHORT nEndRow ) const;

    void        SetDirty();
    void        SetDirty( const ScRange& );
    void        SetDirtyVar();
    void        SetDirtyAfterLoad();
    void        CalcAll();
    void        CalcAfterLoad();
    void        CompileAll();
    void        CompileXML();

    void        ResetChanged( USHORT nStartRow, USHORT nEndRow );

    void        UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                     USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                     short nDx, short nDy, short nDz,
                                     ScDocument* pUndoDoc = NULL );
    void        UpdateInsertTab( USHORT nTable);
    void        UpdateInsertTabOnlyCells( USHORT nTable);
    void        UpdateDeleteTab( USHORT nTable, BOOL bIsMove, ScColumn* pRefUndo = NULL );
    void        UpdateMoveTab(USHORT nOldPos, USHORT nNewPos, USHORT nTabNo);
    void        UpdateCompile( BOOL bForceIfNameInUse = FALSE );
    void        UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc );
    void        UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY );

    void        SetTabNo(USHORT nNewTab);
    BOOL        IsRangeNameInUse(USHORT nRow1, USHORT nRow2, USHORT nIndex) const;
    void        ReplaceRangeNamesInUse( USHORT nRow1, USHORT nRow2, const ScIndexMap& rMap );

    const SfxPoolItem*      GetAttr( USHORT nRow, USHORT nWhich ) const;
    const ScPatternAttr*    GetPattern( USHORT nRow ) const;
    ULONG       GetNumberFormat( USHORT nRow ) const;

    void        MergeSelectionPattern( SfxItemSet** ppSet, const ScMarkData& rMark, BOOL bDeep ) const;
    void        MergePatternArea( SfxItemSet** ppSet, USHORT nRow1, USHORT nRow2, BOOL bDeep ) const;
    void        MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                            ScLineFlags& rFlags,
                            USHORT nStartRow, USHORT nEndRow, BOOL bLeft, USHORT nDistRight ) const;
    void        ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            USHORT nStartRow, USHORT nEndRow, BOOL bLeft, USHORT nDistRight );

    void        ApplyAttr( USHORT nRow, const SfxPoolItem& rAttr );
    void        ApplyPattern( USHORT nRow, const ScPatternAttr& rPatAttr );
    void        ApplyPatternArea( USHORT nStartRow, USHORT nEndRow, const ScPatternAttr& rPatAttr );
    void        SetPattern( USHORT nRow, const ScPatternAttr& rPatAttr, BOOL bPutToPool = FALSE );
    void        SetPatternArea( USHORT nStartRow, USHORT nEndRow,
                                const ScPatternAttr& rPatAttr, BOOL bPutToPool = FALSE );
    void        ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
                            const ScPatternAttr& rPattern, short nNewType );

    void        ApplyStyle( USHORT nRow, const ScStyleSheet& rStyle );
    void        ApplyStyleArea( USHORT nStartRow, USHORT nEndRow, const ScStyleSheet& rStyle );
    void        ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark);
    void        ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const SvxBorderLine* pLine, BOOL bColorOnly );

    const ScStyleSheet* GetStyle( USHORT nRow ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const;
    const ScStyleSheet* GetAreaStyle( BOOL& rFound, USHORT nRow1, USHORT nRow2 ) const;

    void        FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, BOOL* pUsed, BOOL bReset );
    BOOL        IsStyleSheetUsed( const SfxStyleSheetBase& rStyle ) const;

    short       SearchStyle( short nRow, const ScStyleSheet* pSearchStyle,
                                BOOL bUp, BOOL bInSelection, const ScMarkData& rMark );
    BOOL        SearchStyleRange( short& rRow, short& rEndRow, const ScStyleSheet* pSearchStyle,
                                    BOOL bUp, BOOL bInSelection, const ScMarkData& rMark );

    BOOL        ApplyFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags );
    BOOL        RemoveFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags );

    void        RemoveProtected( USHORT nStartRow, USHORT nEndRow );

    short       ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark );
    void        DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark );

    void        ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark );
    void        ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark );

    long        GetNeededSize( USHORT nRow, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, const ScNeededSizeOptions& rOptions );
    USHORT      GetOptimalColWidth( OutputDevice* pDev, double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bFormula, USHORT nOldWidth,
                                    const ScMarkData* pMarkData,
                                    BOOL bSimpleTextImport );
    void        GetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT* pHeight,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bShrink, USHORT nMinHeight, USHORT nMinStart );
private:
    long        GetSimpleTextNeededSize( USHORT nIndex, OutputDevice* pDev,
                                    BOOL bWidth );
public:

    short       GetNextUnprotected( short nRow, BOOL bUp ) const;   // inkl. aktuelle

    void        GetFilterEntries(USHORT nStartRow, USHORT nEndRow, TypedStrCollection& rStrings);
    BOOL        GetDataEntries(USHORT nRow, TypedStrCollection& rStrings, BOOL bLimit);

    USHORT      NoteCount( USHORT nMaxRow = MAXROW ) const;

    void        LoadData( SvStream& rStream );
    void        SaveData( SvStream& rStream ) const;

    void        LoadNotes( SvStream& rStream );
    void        SaveNotes( SvStream& rStream ) const;

    BOOL        Load( SvStream& rStream, ScMultipleReadHeader& rHdr );
    BOOL        Save( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;
    void        UpdateInsertTabAbs(USHORT nNewPos);
    BOOL        TestTabRefAbs(USHORT nTable);
    BOOL        GetNextSpellingCell(USHORT& nRow, BOOL bInSel, const ScMarkData& rData) const;

    void        RemoveAutoSpellObj();

    void        StartListening( SfxListener& rLst, USHORT nRow );
    void        EndListening( SfxListener& rLst, USHORT nRow );
    void        MoveListeners( ScBroadcasterList& rSource, USHORT nDestRow );
    void        StartAllListeners();
    void        StartRelNameListeners();
    void        SetRelNameDirty();

    void        CompileDBFormula();
    void        CompileDBFormula( BOOL bCreateFormulaString );
    void        CompileNameFormula( BOOL bCreateFormulaString );
    void        CompileColRowNameFormula();

    // maximale Stringlaengen einer Column, fuer z.B. dBase Export
    xub_StrLen  GetMaxStringLen( USHORT nRowStart, USHORT nRowEnd ) const;
    xub_StrLen  GetMaxNumberStringLen( USHORT& nPrecision,
                                    USHORT nRowStart, USHORT nRowEnd ) const;

private:
    ScBaseCell* CloneCell(USHORT nIndex, USHORT nFlags,
                            ScDocument* pDestDoc, const ScAddress& rDestPos);
};


class ScColumnIterator                  // alle Daten eines Bereichs durchgehen
{
    const ScColumn*     pColumn;
    USHORT              nPos;
    USHORT              nTop;
    USHORT              nBottom;
public:
                ScColumnIterator( const ScColumn* pCol, USHORT nStart=0, USHORT nEnd=MAXROW );
                ~ScColumnIterator();

    BOOL        Next( USHORT& rRow, ScBaseCell*& rpCell );
    USHORT      GetIndex() const;
};


class ScMarkedDataIter                  // Daten in selektierten Bereichen durchgehen
{
    const ScColumn*     pColumn;
    USHORT              nPos;
    ScMarkArrayIter*    pMarkIter;
    USHORT              nTop;
    USHORT              nBottom;
    BOOL                bNext;
    BOOL                bAll;

public:
                ScMarkedDataIter( const ScColumn* pCol, const ScMarkData* pMarkData,
                                    BOOL bAllIfNone = FALSE );
                ~ScMarkedDataIter();

    BOOL        Next( USHORT& rIndex );
};


#endif


