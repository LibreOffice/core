/*************************************************************************
 *
 *  $RCSfile: htmlpars.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dr $ $Date: 2001-04-06 12:38:30 $
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

#ifndef SC_HTMLPARS_HXX
#define SC_HTMLPARS_HXX

#ifndef _STACK_HXX //autogen
#include <tools/stack.hxx>
#endif

#include "rangelst.hxx"
#include "eeparser.hxx"

#define SC_HTML_FONTSIZES 7     // wie Export, HTML-Options
#define SC_HTML_OFFSET_TOL 10   // Pixeltoleranz fuer SeekOffset

SV_DECL_VARARR_SORT( ScHTMLColOffset, ULONG, 16, 4);

struct ScHTMLTableStackEntry
{
    ScRangeListRef      xLockedList;
    ScEEParseEntry*     pCellEntry;
    ScHTMLColOffset*    pLocalColOffset;
    ULONG               nFirstTableCell;
    USHORT              nColCnt;
    USHORT              nRowCnt;
    USHORT              nColCntStart;
    USHORT              nMaxCol;
    USHORT              nTable;
    USHORT              nTableWidth;
    USHORT              nColOffset;
    USHORT              nColOffsetStart;
    BOOL                bFirstRow;
                        ScHTMLTableStackEntry( ScEEParseEntry* pE,
                                const ScRangeListRef& rL, ScHTMLColOffset* pTO,
                                ULONG nFTC,
                                USHORT nCol, USHORT nRow,
                                USHORT nStart, USHORT nMax, USHORT nTab,
                                USHORT nTW, USHORT nCO, USHORT nCOS,
                                BOOL bFR )
                            : pCellEntry( pE ), xLockedList( rL ),
                            pLocalColOffset( pTO ),
                            nFirstTableCell( nFTC ),
                            nColCnt( nCol ), nRowCnt( nRow ),
                            nColCntStart( nStart ), nMaxCol( nMax ),
                            nTable( nTab ), nTableWidth( nTW ),
                            nColOffset( nCO ), nColOffsetStart( nCOS ),
                            bFirstRow( bFR )
                            {}
                        ~ScHTMLTableStackEntry() {}
};
DECLARE_STACK( ScHTMLTableStack, ScHTMLTableStackEntry* );

struct ScHTMLAdjustStackEntry
{
    USHORT              nLastCol;
    USHORT              nNextRow;
    USHORT              nCurRow;
                        ScHTMLAdjustStackEntry( USHORT nLCol, USHORT nNRow,
                                USHORT nCRow )
                            : nLastCol( nLCol ), nNextRow( nNRow ),
                            nCurRow( nCRow )
                            {}
};
DECLARE_STACK( ScHTMLAdjustStack, ScHTMLAdjustStackEntry* );


//------------------------------------------------------------------------

enum ScHTMLTableDataKey         { tdCol, tdRow };

class ScHTMLTableDataTable;

class ScHTMLTableData           // data for one HTML table
{
private:
    String                      aTableName;     // <TABLE ID="name">
    Table                       aColCount;      // count of cells per HTML column
    Table                       aRowCount;      // count of cells per HTML row
    USHORT                      nFirstCol;      // first column index
    USHORT                      nFirstRow;      // first row index
    USHORT                      nLastCol;       // last column index
    USHORT                      nLastRow;       // last row index
    USHORT                      nColSpan;       // column spanning of the parent cell
    USHORT                      nRowSpan;       // row spanning of the parent cell
    USHORT                      nDocCol;        // resulting position in ScDoc
    USHORT                      nDocRow;        // resulting position in ScDoc
    BOOL                        bBorderOn;      // table borders

    ScHTMLTableDataTable*       pNestedTables;  // table of nested HTML tables

    inline Table&               GetDataTable( ScHTMLTableDataKey eCRKey )
                                    { return (eCRKey == tdCol) ? aColCount : aRowCount; }
    inline const Table&         GetDataTable( ScHTMLTableDataKey eCRKey ) const
                                    { return (eCRKey == tdCol) ? aColCount : aRowCount; }

    static USHORT               GetCount( const Table& rDataTab, ULONG nIndex );
    static USHORT               GetSize( const Table& rDataTab, ULONG nStart, ULONG nEnd );

    USHORT                      CalcDocPos( ScHTMLTableDataKey eCRKey, ULONG nIndex ) const;
    USHORT                      CalcSpan( ScHTMLTableDataKey eCRKey, ULONG nIndex, USHORT nSpan ) const;

    void                        SetMaxCount( Table& rDataTab, ULONG nIndex, USHORT nCount );
    void                        ChangeDocCoord( short nColDiff, short nRowDiff );

    void                        RecalcSizeDim( ScHTMLTableDataKey eCRKey );

public:
                                ScHTMLTableData(
                                    const String& rTabName,
                                    USHORT _nFirstCol, USHORT _nFirstRow,
                                    USHORT _nColSpan, USHORT _nRowSpan,
                                    BOOL bBorder );
                                ~ScHTMLTableData();

    inline USHORT               GetFirstCol() const     { return nFirstCol; }
    inline USHORT               GetFirstRow() const     { return nFirstRow; }
    inline USHORT               GetLastCol() const      { return nLastCol; }
    inline USHORT               GetLastRow() const      { return nLastRow; }
    inline const String&        GetTableName() const    { return aTableName; }
    void                        GetRange( ScRange& rRange ) const;

    inline USHORT               GetFirst( ScHTMLTableDataKey eCRKey ) const
                                    { return (eCRKey == tdCol) ? nFirstCol : nFirstRow; }
    inline USHORT               GetLast( ScHTMLTableDataKey eCRKey ) const
                                    { return (eCRKey == tdCol) ? nLastCol : nLastRow; }
    inline USHORT               GetDocPos( ScHTMLTableDataKey eCRKey ) const
                                    { return (eCRKey == tdCol) ? nDocCol : nDocRow; }
    inline USHORT               GetSpan( ScHTMLTableDataKey eCRKey ) const
                                    { return (eCRKey == tdCol) ? nColSpan : nRowSpan; }
    inline USHORT               GetCount( ScHTMLTableDataKey eCRKey, ULONG nIndex ) const
                                    { return GetCount( GetDataTable( eCRKey ), nIndex ); }
    inline USHORT               GetSize( ScHTMLTableDataKey eCRKey ) const
                                    { return GetSize( GetDataTable( eCRKey ), GetFirst( eCRKey ), GetLast( eCRKey ) ); }

    inline USHORT               CalcDocCol( ULONG nIndex ) const    { return CalcDocPos( tdCol, nIndex ); }
    inline USHORT               CalcDocRow( ULONG nIndex ) const    { return CalcDocPos( tdRow, nIndex ); }
    inline USHORT               CalcColSpan( ULONG nIndex, USHORT nSpan ) const { return CalcSpan( tdCol, nIndex, nSpan ); }
    inline USHORT               CalcRowSpan( ULONG nIndex, USHORT nSpan ) const { return CalcSpan( tdRow, nIndex, nSpan ); }

    ScHTMLTableData*            GetNestedTable( ULONG nTab ) const;

    ScHTMLTableData*            InsertNestedTable(
                                    ULONG nTab, const String& rTabName,
                                    USHORT _nFirstCol, USHORT _nFirstRow,
                                    USHORT _nColSpan, USHORT _nRowSpan,
                                    BOOL bBorder );
    inline void                 SetCellCoord( USHORT nCol, USHORT nRow );
    void                        SetDocCoord( USHORT nCol, USHORT nRow );

    void                        RecalcSize();
    void                        SetCellBorders( ScDocument* pDoc, const ScAddress& rFirstPos );
};

inline void ScHTMLTableData::SetCellCoord( USHORT nCol, USHORT nRow )
{
    nLastCol = Max( nLastCol, nCol );
    nLastRow = Max( nLastRow, nRow );
}



class ScHTMLTableDataTable : private Table
{
private:
    ScHTMLTableData*            pCurrTab;
    ULONG                       nCurrTab;

    USHORT                      nSearchMin1;
    USHORT                      nSearchMin2;
    ScHTMLTableDataKey          eSearchKey;

    inline void                 _Insert( ULONG nTab, ScHTMLTableData* pTable )
                                            { Table::Insert( nTab, pTable ); }

    inline ScHTMLTableData*     _Get( ULONG nTab ) const
                                            { return (ScHTMLTableData*) Table::Get( nTab ); }

    inline void                 SetCurrTable( ULONG nTab, ScHTMLTableData* pTab )
                                            { nCurrTab = nTab; pCurrTab = pTab; }
public:
                                ScHTMLTableDataTable();
    virtual                     ~ScHTMLTableDataTable();

    ScHTMLTableData*            GetTable( ULONG nTab );
    inline ScHTMLTableData*     GetFirst()  { return (ScHTMLTableData*) Table::First(); }
    inline ScHTMLTableData*     GetNext()   { return (ScHTMLTableData*) Table::Next(); }
    ScHTMLTableData*            GetFirstInOrder( ScHTMLTableDataKey eKey );
    ScHTMLTableData*            GetNextInOrder();

    USHORT                      GetNextFreeRow( ULONG nTab );

    ScHTMLTableData*            InsertTable(
                                    ULONG nTab, const String& rTabName,
                                    USHORT nFirstCol, USHORT nFirstRow,
                                    USHORT nColSpan, USHORT nRowSpan,
                                    BOOL bBorder, ULONG nNestedIn = 0 );
    void                        SetCellCoord( ULONG nTab, USHORT nCol, USHORT nRow );
    void                        RecalcSizes();

    void                        Recalc();
    void                        SetCellBorders( ScDocument* pDoc, const ScAddress& rFirstPos );
};

//------------------------------------------------------------------------


class EditEngine;
class ScDocument;
class HTMLOption;

class ScHTMLParser : public ScEEParser
{
private:
    ULONG               aFontHeights[SC_HTML_FONTSIZES];
    ScHTMLTableStack    aTableStack;
    Size                aPageSize;          // in Pixeln
    String              aString;
    ScRangeListRef      xLockedList;        // je Table
    ScDocument*         pDoc;
    Table*              pTables;
    ScHTMLTableDataTable* pTableData;       // data for each HTML table
    ScHTMLColOffset*    pColOffset;
    ScHTMLColOffset*    pLocalColOffset;    // je Table
    ULONG               nFirstTableCell;    // je Table
    short               nTableLevel;
    USHORT              nTable;
    USHORT              nMaxTable;
    USHORT              nColCntStart;       // erste Col je Table
    USHORT              nMaxCol;            // je Table
    USHORT              nTableWidth;        // je Table
    USHORT              nColOffset;         // aktuell, Pixel
    USHORT              nColOffsetStart;    // Startwert je Table, in Pixel
    USHORT              nMetaCnt;           // fuer ParseMetaOptions
    BOOL                bCalcWidthHeight;   // TRUE: calculate real column width
                                            // FALSE: 1 html-col = 1 sc-col
    BOOL                bTabInTabCell;
    BOOL                bFirstRow;          // je Table, ob in erster Zeile
    BOOL                bInCell;
    BOOL                bInTitle;

    DECL_LINK( HTMLImportHdl, ImportInfo* );
    void                NewActEntry( ScEEParseEntry* );
    void                EntryEnd( ScEEParseEntry*, const ESelection& );
    void                ProcToken( ImportInfo* );
    void                CloseEntry( ImportInfo* );
    void                NextRow(  ImportInfo*  );
    void                SkipLocked( ScEEParseEntry*, BOOL bJoin = TRUE );
    static BOOL         SeekOffset( ScHTMLColOffset*, USHORT nOffset,
                                    USHORT* pCol,
                                    USHORT nOffsetTol = SC_HTML_OFFSET_TOL );
    static void         MakeCol( ScHTMLColOffset*, USHORT& nOffset,
                                USHORT& nWidth,
                                USHORT nOffsetTol = SC_HTML_OFFSET_TOL,
                                USHORT nWidthTol = SC_HTML_OFFSET_TOL );
    static void         MakeColNoRef( ScHTMLColOffset*, USHORT nOffset,
                                USHORT nWidth,
                                USHORT nOffsetTol = SC_HTML_OFFSET_TOL,
                                USHORT nWidthTol = SC_HTML_OFFSET_TOL );
    static void         ModifyOffset( ScHTMLColOffset*, USHORT& nOldOffset,
                                    USHORT& nNewOffset,
                                    USHORT nOffsetTol = SC_HTML_OFFSET_TOL );
    void                Colonize( ScEEParseEntry* );
    USHORT              GetWidth( ScEEParseEntry* );
    void                SetWidths();
    void                Adjust();
    void                AdjustNoWidthHeight();

    USHORT              GetWidthPixel( const HTMLOption* );
    BOOL                IsAtBeginningOfText( ImportInfo* );

    void                TableOn( ImportInfo* );
    void                ColOn( ImportInfo* );
    void                TableDataOn( ImportInfo* );
    void                TableOff( ImportInfo* );
    void                Image( ImportInfo* );
    void                AnchorOn( ImportInfo* );
    void                FontOn( ImportInfo* );

public:
                        ScHTMLParser( EditEngine*, const Size& aPageSize,
                            ScDocument*, BOOL _bCalcWidthHeight );
    virtual             ~ScHTMLParser();
    virtual ULONG       Read( SvStream& );

    ScHTMLTableDataTable* GetHTMLTables() const { return pTableData; }
};


#endif

