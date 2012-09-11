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

#ifndef SC_HTMLPARS_HXX
#define SC_HTMLPARS_HXX

#include <memory>
#include <stack>
#include <vector>
#include <list>
#include <map>
#include <o3tl/sorted_vector.hxx>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/unordered_map.hpp>

#include "rangelst.hxx"
#include "eeparser.hxx"

const sal_uInt32 SC_HTML_FONTSIZES = 7;        // wie Export, HTML-Options

// Pixel tolerance for SeekOffset and related.
const sal_uInt16 SC_HTML_OFFSET_TOLERANCE_SMALL = 1;    // single table
const sal_uInt16 SC_HTML_OFFSET_TOLERANCE_LARGE = 10;   // nested

// ============================================================================
// BASE class for HTML parser classes
// ============================================================================

class ScHTMLTable;

/**
 * Collection of HTML style data parsed from the content of <style>
 * elements.
 */
class ScHTMLStyles
{
    typedef ::boost::unordered_map<rtl::OUString, rtl::OUString, rtl::OUStringHash> PropsType;
    typedef ::boost::ptr_map<rtl::OUString, PropsType> NamePropsType;
    typedef ::boost::ptr_map<rtl::OUString, NamePropsType> ElemsType;

    NamePropsType maGlobalProps;     /// global properties (for a given class for all elements)
    NamePropsType maElemGlobalProps; /// element global properties (no class specified)
    ElemsType maElemProps;           /// element to class to properties (both element and class are given)
    const rtl::OUString maEmpty;     /// just a persistent empty string.
public:
    ScHTMLStyles();

    void add(const char* pElemName, size_t nElemName, const char* pClassName, size_t nClassName,
             const rtl::OUString& aProp, const rtl::OUString& aValue);

    /**
     * Find best-matching property value for given element and class names.
     */
    const rtl::OUString& getPropertyValue(
        const rtl::OUString& rElem, const rtl::OUString& rClass, const rtl::OUString& rPropName) const;

private:
    static void insertProp(
        NamePropsType& rProps, const rtl::OUString& aName,
        const rtl::OUString& aProp, const rtl::OUString& aValue);
};

/** Base class for HTML parser classes. */
class ScHTMLParser : public ScEEParser
{
    ScHTMLStyles                maStyles;
protected:
    sal_uInt32                  maFontHeights[ SC_HTML_FONTSIZES ];
    ScDocument*                 mpDoc;          /// The destination document.

public:
    explicit                    ScHTMLParser( EditEngine* pEditEngine, ScDocument* pDoc );
    virtual                     ~ScHTMLParser();

    virtual sal_uLong               Read( SvStream& rStrm, const String& rBaseURL  ) = 0;

    ScHTMLStyles&               GetStyles();
    ScDocument&                 GetDoc();

    /** Returns the "global table" which contains the entire HTML document. */
    virtual const ScHTMLTable*  GetGlobalTable() const = 0;
};


// ============================================================================

typedef o3tl::sorted_vector<sal_uLong> ScHTMLColOffset;

struct ScHTMLTableStackEntry
{
    ScRangeListRef      xLockedList;
    ScEEParseEntry*     pCellEntry;
    ScHTMLColOffset*    pLocalColOffset;
    sal_uLong               nFirstTableCell;
    SCCOL               nColCnt;
    SCROW               nRowCnt;
    SCCOL               nColCntStart;
    SCCOL               nMaxCol;
    sal_uInt16              nTable;
    sal_uInt16              nTableWidth;
    sal_uInt16              nColOffset;
    sal_uInt16              nColOffsetStart;
    bool                bFirstRow;
                        ScHTMLTableStackEntry( ScEEParseEntry* pE,
                                const ScRangeListRef& rL, ScHTMLColOffset* pTO,
                                sal_uLong nFTC,
                                SCCOL nCol, SCROW nRow,
                                SCCOL nStart, SCCOL nMax, sal_uInt16 nTab,
                                sal_uInt16 nTW, sal_uInt16 nCO, sal_uInt16 nCOS,
                                bool bFR )
                            : xLockedList( rL ), pCellEntry( pE ),
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
typedef ::std::stack< ScHTMLTableStackEntry* > ScHTMLTableStack;

struct ScHTMLAdjustStackEntry
{
    SCCOL               nLastCol;
    SCROW               nNextRow;
    SCROW               nCurRow;
                        ScHTMLAdjustStackEntry( SCCOL nLCol, SCROW nNRow,
                                SCROW nCRow )
                            : nLastCol( nLCol ), nNextRow( nNRow ),
                            nCurRow( nCRow )
                            {}
};
typedef ::std::stack< ScHTMLAdjustStackEntry* > ScHTMLAdjustStack;


// ============================================================================

class EditEngine;
class ScDocument;
class HTMLOption;

// TODO these need better names
typedef ::std::map<SCROW, SCROW> InnerMap;
typedef ::std::map<sal_uInt16, InnerMap*> OuterMap;

class ScHTMLLayoutParser : public ScHTMLParser
{
private:
    Size                aPageSize;
    rtl::OUString       aBaseURL;
    ScHTMLTableStack    aTableStack;
    rtl::OUString       aString;
    ScRangeListRef      xLockedList;        // je Table
    OuterMap*           pTables;
    ScHTMLColOffset*    pColOffset;
    ScHTMLColOffset*    pLocalColOffset;    // je Table
    sal_uLong               nFirstTableCell;    // je Table
    short               nTableLevel;
    sal_uInt16              nTable;
    sal_uInt16              nMaxTable;
    SCCOL               nColCntStart;       // erste Col je Table
    SCCOL               nMaxCol;            // je Table
    sal_uInt16              nTableWidth;        // je Table
    sal_uInt16              nColOffset;         // aktuell, Pixel
    sal_uInt16              nColOffsetStart;    // Startwert je Table, in Pixel
    sal_uInt16              nOffsetTolerance;   // for use with SeekOffset and related
    bool                bTabInTabCell:1;
    bool                bFirstRow:1;          // je Table, ob in erster Zeile
    bool                bInCell:1;
    bool                bInTitle:1;

    DECL_LINK( HTMLImportHdl, ImportInfo* );
    void                NewActEntry( ScEEParseEntry* );
    void                EntryEnd( ScEEParseEntry*, const ESelection& );
    void                ProcToken( ImportInfo* );
    void                CloseEntry( ImportInfo* );
    void                NextRow(  ImportInfo*  );
    void                SkipLocked( ScEEParseEntry*, bool bJoin = true );
    static bool         SeekOffset( ScHTMLColOffset*, sal_uInt16 nOffset,
                                    SCCOL* pCol, sal_uInt16 nOffsetTol );
    static void         MakeCol( ScHTMLColOffset*, sal_uInt16& nOffset,
                                sal_uInt16& nWidth, sal_uInt16 nOffsetTol,
                                sal_uInt16 nWidthTol );
    static void         MakeColNoRef( ScHTMLColOffset*, sal_uInt16 nOffset,
                                sal_uInt16 nWidth, sal_uInt16 nOffsetTol,
                                sal_uInt16 nWidthTol );
    static void         ModifyOffset( ScHTMLColOffset*, sal_uInt16& nOldOffset,
                                    sal_uInt16& nNewOffset, sal_uInt16 nOffsetTol );
    void                Colonize( ScEEParseEntry* );
    sal_uInt16              GetWidth( ScEEParseEntry* );
    void                SetWidths();
    void                Adjust();

    sal_uInt16              GetWidthPixel( const HTMLOption& );
    bool                IsAtBeginningOfText( ImportInfo* );

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
                        ScHTMLLayoutParser( EditEngine*, const String& rBaseURL, const Size& aPageSize, ScDocument* );
    virtual             ~ScHTMLLayoutParser();
    virtual sal_uLong       Read( SvStream&, const String& rBaseURL  );
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
    SCCOL               mnCol;
    SCROW               mnRow;

    inline explicit     ScHTMLPos() : mnCol( 0 ), mnRow( 0 ) {}
    inline explicit     ScHTMLPos( SCCOL nCol, SCROW nRow ) :
                            mnCol( nCol ), mnRow( nRow ) {}
    inline explicit     ScHTMLPos( const ScAddress& rAddr ) { Set( rAddr ); }

    inline SCCOLROW     Get( ScHTMLOrient eOrient ) const
                            { return (eOrient == tdCol) ? mnCol : mnRow; }
    inline void         Set( SCCOL nCol, SCROW nRow )
                            { mnCol = nCol; mnRow = nRow; }
    inline void         Set( const ScAddress& rAddr )
                            { Set( rAddr.Col(), rAddr.Row() ); }
    inline void         Move( SCsCOL nColDiff, SCsROW nRowDiff )
                            { mnCol = mnCol + nColDiff; mnRow = mnRow + nRowDiff; }
    inline ScAddress    MakeAddr() const
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
    SCCOL               mnCols;
    SCROW               mnRows;

    inline explicit     ScHTMLSize() : mnCols( 0 ), mnRows( 0 ) {}
    inline explicit     ScHTMLSize( SCCOL nCols, SCROW nRows ) :
                            mnCols( nCols ), mnRows( nRows ) {}

    inline SCCOLROW     Get( ScHTMLOrient eOrient ) const
                            { return (eOrient == tdCol) ? mnCols : mnRows; }
    inline void         Set( SCCOL nCols, SCROW nRows )
                            { mnCols = nCols; mnRows = nRows; }
    inline void         Expand( SCsCOL nColDiff, SCsROW nRowDiff )
                            { mnCols = mnCols + nColDiff; mnRows = mnRows + nRowDiff; }
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
    explicit            ScHTMLEntry(
                            const SfxItemSet& rItemSet,
                            ScHTMLTableId nTableId = SC_HTML_NO_TABLE );

    /** Returns true, if the selection of the entry is empty. */
    inline bool         IsEmpty() const { return !aSel.HasRange(); }
    /** Returns true, if the entry has any content to be imported. */
    bool                HasContents() const;
    /** Returns true, if the entry represents a table. */
    inline bool         IsTable() const { return nTab != SC_HTML_NO_TABLE; }
    /** Returns true, if the entry represents a table. */
    inline ScHTMLTableId GetTableId() const { return nTab; }

    /** Sets or cleares the import always state. */
    inline void         SetImportAlways( bool bSet = true ) { mbImportAlways = bSet; }
    /** Sets start point of the entry selection to the start of the import info object. */
    void                AdjustStart( const ImportInfo& rInfo );
    /** Sets end point of the entry selection to the end of the import info object. */
    void                AdjustEnd( const ImportInfo& rInfo );
    /** Deletes leading and trailing empty paragraphs from the entry. */
    void                Strip( const EditEngine& rEditEngine );

    /** Returns read/write access to the item set of this entry. */
    inline SfxItemSet&  GetItemSet() { return aItemSet; }
    /** Returns read-only access to the item set of this entry. */
    inline const SfxItemSet& GetItemSet() const { return aItemSet; }

private:
    bool                mbImportAlways;     /// true = Always import this entry.
};

// ============================================================================

/** This struct handles creation of unique table identifiers. */
struct ScHTMLTableAutoId
{
    const ScHTMLTableId mnTableId;          /// The created unique table identifier.
    ScHTMLTableId&      mrnUnusedId;        /// Reference to global unused identifier variable.

    /** The constructor assigns an unused identifier to member mnTableId. */
    explicit            ScHTMLTableAutoId( ScHTMLTableId& rnUnusedId );
};

// ----------------------------------------------------------------------------

class ScHTMLTableMap;

/** Stores data for one table in an HTML document.

    This class does the main work for importing an HTML document. It manages
    the correct insertion of parse entries into the correct cells and the
    creation of nested tables. Recalculation of resulting document size and
    position is done recursively in all nested tables.
 */
class ScHTMLTable
{
public:
    /** Creates a new HTML table without content.
        @descr  Internally handles a current cell position. This position is
            invalid until first calls of RowOn() and DataOn().
        @param rParentTable  Reference to the parent table that owns this table.
        @param bPreFormText  true = Table is based on preformatted text (<pre> tag). */
    explicit            ScHTMLTable(
                            ScHTMLTable& rParentTable,
                            const ImportInfo& rInfo,
                            bool bPreFormText );

    virtual             ~ScHTMLTable();

    /** Returns the name of the table, specified in the TABLE tag. */
    inline const rtl::OUString& GetTableName() const { return maTableName; }
    /** Returns the unique identifier of the table. */
    inline ScHTMLTableId GetTableId() const { return maTableId.mnTableId; }
    /** Returns the table size. */
    inline const ScHTMLSize& GetSize() const { return maSize; }
    /** Returns the cell spanning of the specified cell. */
    ScHTMLSize          GetSpan( const ScHTMLPos& rCellPos ) const;

    /** Searches in all nested tables for the specified table.
        @param nTableId  Unique identifier of the table. */
    ScHTMLTable*        FindNestedTable( ScHTMLTableId nTableId ) const;

    /** Puts the item into the item set of the current entry. */
    void                PutItem( const SfxPoolItem& rItem );
    /** Inserts a text portion into current entry. */
    void                PutText( const ImportInfo& rInfo );
    /** Inserts a new line, if in preformatted text, else does nothing. */
    void                InsertPara( const ImportInfo& rInfo );

    /** Inserts a line break (<br> tag).
        @descr  Inserts the current entry regardless if it is empty. */
    void                BreakOn();
    /** Inserts a heading line (<p> and <h*> tags). */
    void                HeadingOn();
    /** Processes a hyperlink (<a> tag). */
    void                AnchorOn();

    /** Starts a *new* table nested in this table (<table> tag).
        @return  Pointer to the new table. */
    ScHTMLTable*        TableOn( const ImportInfo& rInfo );
    /** Closes *this* table (</table> tag).
        @return  Pointer to the parent table. */
    ScHTMLTable*        TableOff( const ImportInfo& rInfo );
    /** Starts a *new* table based on preformatted text (<pre> tag).
        @return  Pointer to the new table. */
    ScHTMLTable*        PreOn( const ImportInfo& rInfo );
    /** Closes *this* table based on preformatted text (</pre> tag).
        @return  Pointer to the parent table. */
    ScHTMLTable*        PreOff( const ImportInfo& rInfo );

    /** Starts next row (<tr> tag).
        @descr  Cell address is invalid until first call of DataOn(). */
    void                RowOn( const ImportInfo& rInfo );
    /** Closes the current row (<tr> tag).
        @descr  Cell address is invalid until call of RowOn() and DataOn(). */
    void                RowOff( const ImportInfo& rInfo );
    /** Starts the next cell (<td> or <th> tag). */
    void                DataOn( const ImportInfo& rInfo );
    /** Closes the current cell (</td> or </th> tag).
        @descr  Cell address is invalid until next call of DataOn(). */
    void                DataOff( const ImportInfo& rInfo );

    /** Starts the body of the HTML document (<body> tag). */
    void                BodyOn( const ImportInfo& rInfo );
    /** Closes the body of the HTML document (</body> tag). */
    void                BodyOff( const ImportInfo& rInfo );

    /** Closes *this* table (</table> tag) or preformatted text (</pre> tag).
        @descr  Used to close this table object regardless on opening tag type.
        @return  Pointer to the parent table, or this, if no parent found. */
    ScHTMLTable*        CloseTable( const ImportInfo& rInfo );

    /** Returns the resulting document row/column count of the specified HTML row/column. */
    SCCOLROW            GetDocSize( ScHTMLOrient eOrient, SCCOLROW nCellPos ) const;
    /** Returns the resulting document row/column count in the half-open range [nCellBegin, nCellEnd). */
    SCCOLROW            GetDocSize( ScHTMLOrient eOrient, SCCOLROW nCellBegin, SCCOLROW nCellEnd ) const;
    /** Returns the total document row/column count in the specified direction. */
    SCCOLROW            GetDocSize( ScHTMLOrient eOrient ) const;
    /** Returns the total document row/column count of the specified HTML cell. */
    ScHTMLSize          GetDocSize( const ScHTMLPos& rCellPos ) const;

    /** Returns the resulting Calc position of the top left edge of the table. */
    inline const ScHTMLPos& GetDocPos() const { return maDocBasePos; }
    /** Calculates the resulting Calc position of the specified HTML column/row. */
    SCCOLROW            GetDocPos( ScHTMLOrient eOrient, SCCOLROW nCellPos = 0 ) const;
    /** Calculates the resulting Calc position of the specified HTML cell. */
    ScHTMLPos           GetDocPos( const ScHTMLPos& rCellPos ) const;

    /** Calculates the current Calc document area of this table. */
    void                GetDocRange( ScRange& rRange ) const;

    /** Applies border formatting to the passed document. */
    void                ApplyCellBorders( ScDocument* pDoc, const ScAddress& rFirstPos ) const;

    SvNumberFormatter* GetFormatTable();

protected:
    /** Creates a new HTML table without parent.
        @descr  This constructor is used to create the "global table". */
    explicit            ScHTMLTable(
                            SfxItemPool& rPool,
                            EditEngine& rEditEngine,
                            ::std::vector< ScEEParseEntry* >& rEEParseList,
                            ScHTMLTableId& rnUnusedId, ScHTMLParser* pParser );

    /** Fills all empty cells in this and nested tables with dummy parse entries. */
    void                FillEmptyCells();
    /** Recalculates the size of all columns/rows in the table, regarding nested tables. */
    void                RecalcDocSize();
    /** Recalculates the position of all cell entries and nested tables.
        @param rBasePos  The origin of the table in the Calc document. */
    void                RecalcDocPos( const ScHTMLPos& rBasePos );

private:
    typedef ::std::auto_ptr< ScHTMLTableMap >           ScHTMLTableMapPtr;
    typedef ::std::auto_ptr< SfxItemSet >               SfxItemSetPtr;
    typedef ::std::vector< SCCOLROW >                   ScSizeVec;
    typedef ::std::list< ScHTMLEntry* >                 ScHTMLEntryList;
    typedef ::std::map< ScHTMLPos, ScHTMLEntryList >    ScHTMLEntryMap;
    typedef ::std::auto_ptr< ScHTMLEntry >              ScHTMLEntryPtr;

    /** Returns true, if the current cell does not contain an entry yet. */
    bool                IsEmptyCell() const;
    /** Returns the item set from cell, row, or table, depending on current state. */
    const SfxItemSet&   GetCurrItemSet() const;

    /** Returns true, if import info represents a space character. */
    static bool         IsSpaceCharInfo( const ImportInfo& rInfo );

    /** Creates and returns a new empty flying entry at position (0,0). */
    ScHTMLEntryPtr      CreateEntry() const;
    /** Creates a new flying entry.
        @param rInfo  Contains the initial edit engine selection for the entry. */
    void                CreateNewEntry( const ImportInfo& rInfo );

    /** Inserts an empty line in front of the next entry. */
    void                InsertLeadingEmptyLine();

    /** Pushes the passed entry into the list of the current cell. */
    void                ImplPushEntryToList( ScHTMLEntryList& rEntryList, ScHTMLEntryPtr& rxEntry );
    /** Tries to insert the entry into the current cell.
        @descr  If insertion is not possible (i.e., currently no cell open), the
        entry will be inserted into the parent table.
        @return  true = Entry as been pushed into the current cell; false = Entry dropped. */
    bool                PushEntry( ScHTMLEntryPtr& rxEntry );
    /** Puts the current entry into the entry list, if it is not empty.
        @param rInfo  The import info struct containing the end position of the current entry.
        @param bLastInCell  true = If cell is still empty, put this entry always.
        @return  true = Entry as been pushed into the current cell; false = Entry dropped. */
    bool                PushEntry( const ImportInfo& rInfo, bool bLastInCell = false );
    /** Pushes a new entry into current cell which references a nested table.
        @return  true = Entry as been pushed into the current cell; false = Entry dropped. */
    bool                PushTableEntry( ScHTMLTableId nTableId );

    /** Tries to find a table from the table container.
        @descr  Assumes that the table is located in the current container or
        that the passed table identifier is 0.
        @param nTableId  Unique identifier of the table or 0. */
    ScHTMLTable*        GetExistingTable( ScHTMLTableId nTableId ) const;
    /** Inserts a nested table in the current cell at the specified position.
        @param bPreFormText  true = New table is based on preformatted text (<pre> tag). */
    ScHTMLTable*        InsertNestedTable( const ImportInfo& rInfo, bool bPreFormText );

    /** Inserts a new cell in an unused position, starting from current cell position. */
    void                InsertNewCell( const ScHTMLSize& rSpanSize );

    /** Set internal states for a new table row. */
    void                ImplRowOn();
    /** Set internal states for leaving a table row. */
    void                ImplRowOff();
    /** Set internal states for entering a new table cell. */
    void                ImplDataOn( const ScHTMLSize& rSpanSize );
    /** Set internal states for leaving a table cell. */
    void                ImplDataOff();

    /** Inserts additional formatting options from import info into the item set. */
    void                ProcessFormatOptions( SfxItemSet& rItemSet, const ImportInfo& rInfo );

    /** Updates the document column/row size of the specified column or row.
        @descr  Only increases the present count, never decreases. */
    void                SetDocSize( ScHTMLOrient eOrient, SCCOLROW nCellPos, SCCOLROW nSize );
    /** Calculates and sets the resulting size the cell needs in the document.
        @descr  Reduces the needed size in merged cells.
        @param nCellPos  The first column/row position of the (merged) cell.
        @param nCellSpan  The cell spanning in the specified orientation.
        @param nRealDocSize  The raw document size of all entries of the cell. */
    void                CalcNeededDocSize(
                            ScHTMLOrient eOrient, SCCOLROW nCellPos,
                            SCCOLROW nCellSpan, SCCOLROW nRealDocSize );

private:
    ScHTMLTable*        mpParentTable;      /// Pointer to parent table.
    ScHTMLTableMapPtr   mxNestedTables;     /// Table of nested HTML tables.
    rtl::OUString       maTableName;        /// Table name from <table id> option.
    ScHTMLTableAutoId   maTableId;          /// Unique identifier of this table.
    SfxItemSet          maTableItemSet;     /// Items for the entire table.
    SfxItemSetPtr       mxRowItemSet;       /// Items for the current table row.
    SfxItemSetPtr       mxDataItemSet;      /// Items for the current cell.
    ScRangeList         maHMergedCells;     /// List of all horizontally merged cells.
    ScRangeList         maVMergedCells;     /// List of all vertically merged cells.
    ScRangeList         maUsedCells;        /// List of all used cells.
    EditEngine&         mrEditEngine;       /// Edit engine (from ScEEParser).
    ::std::vector< ScEEParseEntry* >& mrEEParseList;      /// List that owns the parse entries (from ScEEParser).
    ScHTMLEntryMap      maEntryMap;         /// List of entries for each cell.
    ScHTMLEntryList*    mpCurrEntryList;    /// Current entry list from map for faster access.
    ScHTMLEntryPtr      mxCurrEntry;        /// Working entry, not yet inserted in a list.
    ScSizeVec           maCumSizes[ 2 ];    /// Cumulated cell counts for each HTML table column/row.
    ScHTMLSize          maSize;             /// Size of the table.
    ScHTMLPos           maCurrCell;         /// Address of current cell to fill.
    ScHTMLPos           maDocBasePos;       /// Resulting base address in a Calc document.
    ScHTMLParser*       mpParser;
    bool                mbBorderOn:1;       /// true = Table borders on.
    bool                mbPreFormText:1;    /// true = Table from preformatted text (<pre> tag).
    bool                mbRowOn:1;          /// true = Inside of <tr> </tr>.
    bool                mbDataOn:1;         /// true = Inside of <td> </td> or <th> </th>.
    bool                mbPushEmptyLine:1;  /// true = Insert empty line before current entry.
};

// ----------------------------------------------------------------------------

/** The "global table" representing the entire HTML document. */
class ScHTMLGlobalTable : public ScHTMLTable
{
public:
    explicit            ScHTMLGlobalTable(
                            SfxItemPool& rPool,
                            EditEngine& rEditEngine,
                            ::std::vector< ScEEParseEntry* >& rEEParseList,
                            ScHTMLTableId& rnUnusedId, ScHTMLParser* pParser );

    virtual             ~ScHTMLGlobalTable();

    /** Recalculates sizes and resulting positions of all document entries. */
    void                Recalc();
};

// ============================================================================

/** The HTML parser for data queries. Focuses on data import, not on layout.

    Builds the table structure correctly, ignores extended formatting like
    pictures or column widths.
 */
class ScHTMLQueryParser : public ScHTMLParser
{
public:
    explicit            ScHTMLQueryParser( EditEngine* pEditEngine, ScDocument* pDoc );
    virtual             ~ScHTMLQueryParser();

    virtual sal_uLong       Read( SvStream& rStrm, const String& rBaseURL  );

    /** Returns the "global table" which contains the entire HTML document. */
    virtual const ScHTMLTable* GetGlobalTable() const;

private:
    /** Handles all possible tags in the HTML document. */
    void                ProcessToken( const ImportInfo& rInfo );
    /** Inserts a text portion into current entry. */
    void                InsertText( const ImportInfo& rInfo );
    /** Processes the <font> tag. */
    void                FontOn( const ImportInfo& rInfo );

    /** Processes the <meta> tag. */
    void                MetaOn( const ImportInfo& rInfo );
    /** Opens the title of the HTML document (<title> tag). */
    void                TitleOn( const ImportInfo& rInfo );
    /** Closes the title of the HTML document (</title> tag). */
    void                TitleOff( const ImportInfo& rInfo );

    /** Opens a new table at the current position. */
    void                TableOn( const ImportInfo& rInfo );
    /** Closes the current table. */
    void                TableOff( const ImportInfo& rInfo );
    /** Opens a new table based on preformatted text. */
    void                PreOn( const ImportInfo& rInfo );
    /** Closes the current preformatted text table. */
    void                PreOff( const ImportInfo& rInfo );

    /** Closes the current table, regardless on opening tag. */
    void                CloseTable( const ImportInfo& rInfo );

    void                ParseStyle(const rtl::OUString& rStrm);

    DECL_LINK( HTMLImportHdl, const ImportInfo* );

private:
    typedef ::std::auto_ptr< ScHTMLGlobalTable >    ScHTMLGlobalTablePtr;

    rtl::OUStringBuffer maTitle;            /// The title of the document.
    ScHTMLGlobalTablePtr mxGlobTable;       /// Contains the entire imported document.
    ScHTMLTable*        mpCurrTable;        /// Pointer to current table (performance).
    ScHTMLTableId       mnUnusedId;         /// First unused table identifier.
    bool                mbTitleOn;          /// true = Inside of <title> </title>.
};


// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
