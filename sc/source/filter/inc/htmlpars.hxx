/*************************************************************************
 *
 *  $RCSfile: htmlpars.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:38:47 $
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

#include <memory>
#include <vector>
#include <list>
#include <map>

#include "rangelst.hxx"
#include "eeparser.hxx"

const sal_uInt32 SC_HTML_FONTSIZES = 7;        // wie Export, HTML-Options
#define SC_HTML_OFFSET_TOL 10   // Pixeltoleranz fuer SeekOffset

// ============================================================================
// BASE class for HTML parser classes
// ============================================================================

class ScHTMLTable;

/** Base class for HTML parser classes. */
class ScHTMLParser : public ScEEParser
{
protected:
    sal_uInt32                  maFontHeights[ SC_HTML_FONTSIZES ];
    ScDocument*                 mpDoc;          /// The destination document.

public:
    explicit                    ScHTMLParser( EditEngine* pEditEngine, ScDocument* pDoc );
    virtual                     ~ScHTMLParser();

    virtual sal_uInt32          Read( SvStream& rStrm ) = 0;

    /** Returns the "global table" which contains the entire HTML document. */
    virtual const ScHTMLTable*  GetGlobalTable() const = 0;
};


// ============================================================================

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


// ============================================================================

class EditEngine;
class ScDocument;
class HTMLOption;

class ScHTMLLayoutParser : public ScHTMLParser
{
private:
    Size                aPageSize;
    ScHTMLTableStack    aTableStack;
    String              aString;
    ScRangeListRef      xLockedList;        // je Table
    Table*              pTables;
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

    USHORT              GetWidthPixel( const HTMLOption* );
    BOOL                IsAtBeginningOfText( ImportInfo* );

    void                TableOn( ImportInfo* );
    void                ColOn( ImportInfo* );
    void                TableRowOn( ImportInfo* );
    void                TableRowOff( ImportInfo* );
    void                TableDataOn( ImportInfo* );
    void                TableDataOff( ImportInfo* );
    void                TableOff( ImportInfo* );
    void                Image( ImportInfo* );
    void                AnchorOn( ImportInfo* );
    void                FontOn( ImportInfo* );

public:
                        ScHTMLLayoutParser( EditEngine*, const Size& aPageSize, ScDocument* );
    virtual             ~ScHTMLLayoutParser();
    virtual ULONG       Read( SvStream& );
    virtual const ScHTMLTable*  GetGlobalTable() const;
};



// ============================================================================
// HTML DATA QUERY PARSER
// ============================================================================

/** Declares the orientation in or for a table: column or row. */
enum ScHTMLOrient { tdCol = 0 , tdRow = 1 };

/** Type for a unique identifier for each table. */
typedef sal_uInt16 ScHTMLTableId;
/** Identifier of the "global table" (the entire HTML document). */
const ScHTMLTableId SC_HTML_GLOBAL_TABLE = 0;
/** Used as table index for normal (non-table) entries in ScHTMLEntry structs. */
const ScHTMLTableId SC_HTML_NO_TABLE = 0;


// ============================================================================

/** A 2D cell position in an HTML table. */
struct ScHTMLPos
{
    sal_uInt16                  mnCol;
    sal_uInt16                  mnRow;

    inline explicit             ScHTMLPos() : mnCol( 0 ), mnRow( 0 ) {}
    inline explicit             ScHTMLPos( sal_uInt16 nCol, sal_uInt16 nRow ) :
                                    mnCol( nCol ), mnRow( nRow ) {}
    inline explicit             ScHTMLPos( const ScAddress& rAddr ) { Set( rAddr ); }

    inline sal_uInt16           Get( ScHTMLOrient eOrient ) const
                                    { return (eOrient == tdCol) ? mnCol : mnRow; }
    inline void                 Set( sal_uInt16 nCol, sal_uInt16 nRow )
                                    { mnCol = nCol; mnRow = nRow; }
    inline void                 Set( const ScAddress& rAddr )
                                    { Set( rAddr.Col(), rAddr.Row() ); }
    inline void                 Move( sal_Int16 nColDiff, sal_Int16 nRowDiff )
                                    { mnCol += nColDiff; mnRow += nRowDiff; }
    inline ScAddress            MakeAddr() const
                                    { return ScAddress( mnCol, mnRow, 0 ); }
};

inline bool operator==( const ScHTMLPos& rPos1, const ScHTMLPos& rPos2 )
{
    return (rPos1.mnRow == rPos2.mnRow) && (rPos1.mnCol == rPos2.mnCol);
}

inline bool operator<( const ScHTMLPos& rPos1, const ScHTMLPos& rPos2 )
{
    return (rPos1.mnRow < rPos2.mnRow) || ((rPos1.mnRow == rPos2.mnRow) && (rPos1.mnCol < rPos2.mnCol));
}


// ----------------------------------------------------------------------------

/** A 2D cell size in an HTML table. */
struct ScHTMLSize
{
    sal_uInt16                  mnCols;
    sal_uInt16                  mnRows;

    inline explicit             ScHTMLSize() : mnCols( 0 ), mnRows( 0 ) {}
    inline explicit             ScHTMLSize( sal_uInt16 nCols, sal_uInt16 nRows ) :
                                    mnCols( nCols ), mnRows( nRows ) {}

    inline sal_uInt16           Get( ScHTMLOrient eOrient ) const
                                    { return (eOrient == tdCol) ? mnCols : mnRows; }
    inline void                 Set( sal_uInt16 nCols, sal_uInt16 nRows )
                                    { mnCols = nCols; mnRows = nRows; }
    inline void                 Expand( sal_Int16 nColDiff, sal_Int16 nRowDiff )
                                    { mnCols += nColDiff; mnRows += nRowDiff; }
};

inline bool operator==( const ScHTMLSize& rSize1, const ScHTMLSize& rSize2 )
{
    return (rSize1.mnRows == rSize2.mnRows) && (rSize1.mnCols == rSize2.mnCols);
}


// ============================================================================

/** A single entry containing a line of text or representing a table. */
struct ScHTMLEntry : public ScEEParseEntry
{
public:
    explicit                    ScHTMLEntry(
                                    const SfxItemSet& rItemSet,
                                    ScHTMLTableId nTableId = SC_HTML_NO_TABLE );

    /** Returns true, if the selection of the entry is empty. */
    inline bool                 IsEmpty() const { return !aSel.HasRange(); }
    /** Returns true, if the entry has any content to be imported. */
    bool                        HasContents() const;
    /** Returns true, if the entry represents a table. */
    inline bool                 IsTable() const { return nTab != SC_HTML_NO_TABLE; }
    /** Returns true, if the entry represents a table. */
    inline ScHTMLTableId        GetTableId() const { return nTab; }

    /** Sets or cleares the import always state. */
    inline void                 SetImportAlways( bool bSet = true ) { mbImportAlways = bSet; }
    /** Sets start point of the entry selection to the start of the import info object. */
    void                        AdjustStart( const ImportInfo& rInfo );
    /** Sets end point of the entry selection to the end of the import info object. */
    void                        AdjustEnd( const ImportInfo& rInfo );
    /** Deletes leading and trailing empty paragraphs from the entry. */
    void                        Strip( const EditEngine& rEditEngine );

    /** Returns read/write access to the item set of this entry. */
    inline SfxItemSet&          GetItemSet() { return aItemSet; }
    /** Returns read-only access to the item set of this entry. */
    inline const SfxItemSet&    GetItemSet() const { return aItemSet; }

private:
    bool                        mbImportAlways; /// true = Always import this entry.
};


// ============================================================================

/** This struct handles creation of unique table identifiers. */
struct ScHTMLTableAutoId
{
    const ScHTMLTableId         mnTableId;          /// The created unique table identifier.
    ScHTMLTableId&              mrnUnusedId;        /// Reference to global unused identifier variable.

    /** The constructor assigns an unused identifier to member mnTableId. */
    explicit                    ScHTMLTableAutoId( ScHTMLTableId& rnUnusedId );
};


// ----------------------------------------------------------------------------

class ScHTMLTableMap;

/** Stores data for one table in an HTML document.
    @descr  This class does the main work for importing an HTML document. It manages
    the correct insertion of parse entries into the correct cells and the creation
    of nested tables. Recalculation of resulting document size and position is done
    recursively in all nested tables. */
class ScHTMLTable
{
public:
    /** Creates a new HTML table without content.
        @descr  Internally handles a current cell position. This position is invalid
        until first calls of RowOn() and DataOn().
        @param rParentTable  Reference to the parent table that owns this table.
        @param bPreFormText  true = Table is based on preformatted text (<pre> tag). */
    explicit                    ScHTMLTable(
                                    ScHTMLTable& rParentTable,
                                    const ImportInfo& rInfo,
                                    bool bPreFormText );

    virtual                     ~ScHTMLTable();

    /** Returns the name of the table, specified in the TABLE tag. */
    inline const String&        GetTableName() const { return maTableName; }
    /** Returns the unique identifier of the table. */
    inline ScHTMLTableId        GetTableId() const { return maTableId.mnTableId; }
    /** Returns the table size. */
    inline const ScHTMLSize&    GetSize() const { return maSize; }
    /** Returns the cell spanning of the specified cell. */
    ScHTMLSize                  GetSpan( const ScHTMLPos& rCellPos ) const;

    /** Searches in all nested tables for the specified table.
        @param nTableId  Unique identifier of the table. */
    ScHTMLTable*                FindNestedTable( ScHTMLTableId nTableId ) const;

    /** Puts the item into the item set of the current entry. */
    void                        PutItem( const SfxPoolItem& rItem );
    /** Inserts a text portion into current entry. */
    void                        PutText( const ImportInfo& rInfo );
    /** Inserts a new line, if in preformatted text, else does nothing. */
    void                        InsertPara( const ImportInfo& rInfo );

    /** Inserts a line break (<br> tag).
        @descr  Inserts the current entry regardless if it is empty. */
    void                        BreakOn();
    /** Inserts a heading line (<p> and <h*> tags). */
    void                        HeadingOn();
    /** Processes a hyperlink (<a> tag). */
    void                        AnchorOn();

    /** Starts a *new* table nested in this table (<table> tag).
        @return  Pointer to the new table. */
    ScHTMLTable*                TableOn( const ImportInfo& rInfo );
    /** Closes *this* table (</table> tag).
        @return  Pointer to the parent table. */
    ScHTMLTable*                TableOff( const ImportInfo& rInfo );
    /** Starts a *new* table based on preformatted text (<pre> tag).
        @return  Pointer to the new table. */
    ScHTMLTable*                PreOn( const ImportInfo& rInfo );
    /** Closes *this* table based on preformatted text (</pre> tag).
        @return  Pointer to the parent table. */
    ScHTMLTable*                PreOff( const ImportInfo& rInfo );

    /** Starts next row (<tr> tag).
        @descr  Cell address is invalid until first call of DataOn(). */
    void                        RowOn( const ImportInfo& rInfo );
    /** Closes the current row (<tr> tag).
        @descr  Cell address is invalid until call of RowOn() and DataOn(). */
    void                        RowOff( const ImportInfo& rInfo );
    /** Starts the next cell (<td> or <th> tag). */
    void                        DataOn( const ImportInfo& rInfo );
    /** Closes the current cell (</td> or </th> tag).
        @descr  Cell address is invalid until next call of DataOn(). */
    void                        DataOff( const ImportInfo& rInfo );

    /** Starts the body of the HTML document (<body> tag). */
    void                        BodyOn( const ImportInfo& rInfo );
    /** Closes the body of the HTML document (</body> tag). */
    void                        BodyOff( const ImportInfo& rInfo );

    /** Returns the resulting document row/column count of the specified HTML row/column. */
    sal_uInt16                  GetDocSize( ScHTMLOrient eOrient, sal_uInt16 nCellPos ) const;
    /** Returns the resulting document row/column count in the range [nCellBegin, nCellEnd). */
    sal_uInt16                  GetDocSize( ScHTMLOrient eOrient, sal_uInt16 nCellBegin, sal_uInt16 nCellEnd ) const;
    /** Returns the total document row/column count in the specified direction. */
    sal_uInt16                  GetDocSize( ScHTMLOrient eOrient ) const;
    /** Returns the total document row/column count of the specified HTML cell. */
    ScHTMLSize                  GetDocSize( const ScHTMLPos& rCellPos ) const;

    /** Returns the resulting Calc position of the top left edge of the table. */
    inline const ScHTMLPos&     GetDocPos() const { return maDocBasePos; }
    /** Calculates the resulting Calc position of the specified HTML column/row. */
    sal_uInt16                  GetDocPos( ScHTMLOrient eOrient, sal_uInt16 nCellPos = 0 ) const;
    /** Calculates the resulting Calc position of the specified HTML cell. */
    ScHTMLPos                   GetDocPos( const ScHTMLPos& rCellPos ) const;

    /** Calculates the current Calc document area of this table. */
    void                        GetDocRange( ScRange& rRange ) const;

    /** Applies border formatting to the passed document. */
    void                        ApplyCellBorders( ScDocument* pDoc, const ScAddress& rFirstPos ) const;

protected:
    /** Creates a new HTML table without parent.
        @descr  This constructor is used to create the "global table". */
    explicit                    ScHTMLTable(
                                    SfxItemPool& rPool,
                                    EditEngine& rEditEngine,
                                    ScEEParseList& rEEParseList,
                                    ScHTMLTableId& rnUnusedId );

    /** Fills all empty cells in this and nested tables with dummy parse entries. */
    void                        FillEmptyCells();
    /** Recalculates the size of all columns/rows in the table, regarding nested tables. */
    void                        RecalcDocSize();
    /** Recalculates the position of all cell entries and nested tables.
        @param rBasePos  The origin of the table in the Calc document. */
    void                        RecalcDocPos( const ScHTMLPos& rBasePos );

private:
    typedef ::std::auto_ptr< ScHTMLTableMap >           ScHTMLTableMapPtr;
    typedef ::std::auto_ptr< SfxItemSet >               SfxItemSetPtr;
    typedef ::std::vector< sal_uInt16 >                 ScSizeVec;
    typedef ::std::list< ScHTMLEntry* >                 ScHTMLEntryList;
    typedef ::std::map< ScHTMLPos, ScHTMLEntryList >    ScHTMLEntryMap;
    typedef ::std::auto_ptr< ScHTMLEntry >              ScHTMLEntryPtr;

    /** Returns true, if the current cell does not contain an entry yet. */
    bool                        IsEmptyCell() const;
    /** Returns the item set from cell, row, or table, depending on current state. */
    const SfxItemSet&           GetCurrItemSet() const;

    /** Returns true, if import info represents a space character. */
    static bool                 IsSpaceCharInfo( const ImportInfo& rInfo );

    /** Creates and returns a new empty flying entry at position (0,0). */
    ScHTMLEntryPtr              CreateEntry() const;
    /** Creates a new flying entry.
        @param rInfo  Contains the initial edit engine selection for the entry. */
    void                        CreateNewEntry( const ImportInfo& rInfo );

    /** Inserts an empty line in front of the next entry. */
    void                        InsertLeadingEmptyLine();

    /** Pushes the passed entry into the list of the current cell. */
    void                        ImplPushEntryToList( ScHTMLEntryList& rEntryList, ScHTMLEntryPtr& rpEntry );
    /** Tries to insert the entry into the current cell.
        @descr  If insertion is not possible (i.e., currently no cell open), the
        entry will be inserted into the parent table.
        @return  true = Entry as been pushed into the current cell; false = Entry dropped. */
    bool                        PushEntry( ScHTMLEntryPtr& rpEntry );
    /** Puts the current entry into the entry list, if it is not empty.
        @param rInfo  The import info struct containing the end position of the current entry.
        @param bLastInCell  true = If cell is still empty, put this entry always.
        @return  true = Entry as been pushed into the current cell; false = Entry dropped. */
    bool                        PushEntry( const ImportInfo& rInfo, bool bLastInCell = false );
    /** Pushes a new entry into current cell which references a nested table.
        @return  true = Entry as been pushed into the current cell; false = Entry dropped. */
    bool                        PushTableEntry( ScHTMLTableId nTableId );

    /** Tries to find a table from the table container.
        @descr  Assumes that the table is located in the current container or
        that the passed table identifier is 0.
        @param nTableId  Unique identifier of the table or 0. */
    ScHTMLTable*                GetExistingTable( ScHTMLTableId nTableId ) const;
    /** Inserts a nested table in the current cell at the specified position.
        @param bPreFormText  true = New table is based on preformatted text (<pre> tag). */
    ScHTMLTable*                InsertNestedTable( const ImportInfo& rInfo, bool bPreFormText );
    /** Closes this table and inserts it into parent table.
        @return  Pointer to the parent table, or this, if no parent found. */
    ScHTMLTable*                CloseTable( const ImportInfo& rInfo );

    /** Inserts a new cell in an unused position, starting from current cell position. */
    void                        InsertNewCell( const ScHTMLSize& rSpanSize );

    /** Set internal states for a new table row. */
    void                        ImplRowOn();
    /** Set internal states for leaving a table row. */
    void                        ImplRowOff();
    /** Set internal states for entering a new table cell. */
    void                        ImplDataOn( const ScHTMLSize& rSpanSize );
    /** Set internal states for leaving a table cell. */
    void                        ImplDataOff();

    /** Inserts additional formatting options from import info into the item set. */
    void                        ProcessFormatOptions( SfxItemSet& rItemSet, const ImportInfo& rInfo );

    /** Updates the document column/row size of the specified column or row.
        @descr  Only increases the present count, never decreases. */
    void                        SetDocSize( ScHTMLOrient eOrient, sal_uInt16 nCellPos, sal_uInt16 nSize );
    /** Calculates and sets the resulting size the cell needs in the document.
        @descr  Reduces the needed size in merged cells.
        @param nCellPos  The first column/row position of the (merged) cell.
        @param nCellSpan  The cell spanning in the specified orientation.
        @param nRealDocSize  The raw document size of all entries of the cell. */
    void                        CalcNeededDocSize(
                                    ScHTMLOrient eOrient, sal_uInt16 nCellPos,
                                    sal_uInt16 nCellSpan, sal_uInt16 nRealDocSize );

private:
    ScHTMLTable*                mpParentTable;      /// Pointer to parent table.
    ScHTMLTableMapPtr           mpNestedTables;     /// Table of nested HTML tables.
    String                      maTableName;        /// Table name from <table id> option.
    ScHTMLTableAutoId           maTableId;          /// Unique identifier of this table.
    SfxItemSet                  maTableItemSet;     /// Items for the entire table.
    SfxItemSetPtr               mpRowItemSet;       /// Items for the current table row.
    SfxItemSetPtr               mpDataItemSet;      /// Items for the current cell.
    ScRangeList                 maLockList;         /// Locked cells (needed for merged cells).
    EditEngine&                 mrEditEngine;       /// Edit engine (from ScEEParser).
    ScEEParseList&              mrEEParseList;      /// List that owns the parse entries (from ScEEParser).
    ScHTMLEntryMap              maEntryMap;         /// List of entries for each cell.
    ScHTMLEntryList*            mpCurrEntryList;    /// Current entry list from map for faster access.
    ScHTMLEntryPtr              mpCurrEntry;        /// Working entry, not yet inserted in a list.
    ScSizeVec                   maSizes[ 2 ];       /// Calc cell count of each HTML table column/row.
    ScHTMLSize                  maSize;             /// Size of the table.
    ScHTMLPos                   maCurrCell;         /// Address of current cell to fill.
    ScHTMLPos                   maDocBasePos;       /// Resulting base address in a Calc document.
    bool                        mbBorderOn;         /// true = Table borders on.
    bool                        mbPreFormText;      /// true = Table from preformatted text (<pre> tag).
    bool                        mbRowOn;            /// true = Inside of <tr> </tr>.
    bool                        mbDataOn;           /// true = Inside of <td> </td> or <th> </th>.
    bool                        mbPushEmptyLine;    /// true = Insert empty line before current entry.
};


// ----------------------------------------------------------------------------

/** The "global table" representing the entire HTML document. */
class ScHTMLGlobalTable : public ScHTMLTable
{
public:
    explicit                    ScHTMLGlobalTable(
                                    SfxItemPool& rPool,
                                    EditEngine& rEditEngine,
                                    ScEEParseList& rEEParseList,
                                    ScHTMLTableId& rnUnusedId );

    virtual                     ~ScHTMLGlobalTable();

    /** Recalculates sizes and resulting positions of all document entries. */
    void                        Recalc();
};


// ============================================================================

/** The HTML parser for data queries. Focuses on data import, not on layout.
    @descr  Builds the table structure correctly, ignores extended formatting
    like pictures or column widths. */
class ScHTMLQueryParser : public ScHTMLParser
{
public:
    explicit                    ScHTMLQueryParser( EditEngine* pEditEngine, ScDocument* pDoc );
    virtual                     ~ScHTMLQueryParser();

    virtual sal_uInt32          Read( SvStream& rStrm );

    /** Returns the "global table" which contains the entire HTML document. */
    virtual const ScHTMLTable*  GetGlobalTable() const;

private:
    /** Returns the specified table (may be maGlobTable, if no real table can be found). */
    ScHTMLTable*                GetTable( ScHTMLTableId nTableId ) const;

    /** Handles all possible tags in the HTML document. */
    void                        ProcessToken( const ImportInfo& rInfo );
    /** Inserts a text portion into current entry. */
    void                        InsertText( const ImportInfo& rInfo );
    /** Processes the <font> tag. */
    void                        FontOn( const ImportInfo& rInfo );

    /** Processes the <meta> tag. */
    void                        MetaOn( const ImportInfo& rInfo );
    /** Opens the title of the HTML document (<title> tag). */
    void                        TitleOn( const ImportInfo& rInfo );
    /** Closes the title of the HTML document (</title> tag). */
    void                        TitleOff( const ImportInfo& rInfo );

    /** Opens a new table at the current position. */
    void                        TableOn( const ImportInfo& rInfo );
    /** Closes the current table. */
    void                        TableOff( const ImportInfo& rInfo );
    /** Opens a new table based on preformatted text. */
    void                        PreOn( const ImportInfo& rInfo );
    /** Closes the current preformatted text table. */
    void                        PreOff( const ImportInfo& rInfo );

    DECL_LINK( HTMLImportHdl, const ImportInfo* );

private:
    typedef ::std::auto_ptr< ScHTMLGlobalTable >    ScHTMLGlobalTablePtr;

    String                      maTitle;            /// The title of the document.
    ScHTMLGlobalTablePtr        mpGlobTable;        /// Contains the entire imported document.
    ScHTMLTable*                mpCurrTable;        /// Pointer to current table (performance).
    ScHTMLTableId               mnUnusedId;         /// First unused table identifier.
    bool                        mbTitleOn;          /// true = Inside of <title> </title>.
};


// ============================================================================

#endif

