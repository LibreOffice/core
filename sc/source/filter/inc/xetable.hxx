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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XETABLE_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XETABLE_HXX

#include "xltable.hxx"

#include <vector>
#include <tools/mempool.hxx>
#include "xladdress.hxx"
#include "xerecord.hxx"
#include "xestring.hxx"
#include "xeformula.hxx"
#include "xestyle.hxx"
#include "xeextlst.hxx"

#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>

/* ============================================================================
Export of cell tables including row and column description.
- Managing all used and formatted cells in a sheet.
- Row and column properties, i.e. width/height, visibility.
- Find default row formatting and default column formatting.
- Merged cell ranges.
============================================================================ */

// Helper records for cell records

/** Represents a STRING record that contains the result of a string formula. */
class XclExpStringRec : public XclExpRecord
{
public:
    explicit            XclExpStringRec( const XclExpRoot& rRoot, const OUString& rResult );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclExpStringRef     mxResult;
};

// Additional records for special formula ranges

/** Base record for additional range formula records (i.e. ARRAY, SHRFMLA). */
class XclExpRangeFmlaBase : public XclExpRecord
{
public:
    /** Returns true, if the passed cell position is equal to own base position. */
    bool                IsBasePos( sal_uInt16 nXclCol, sal_uInt32 nXclRow ) const;

    /** Derived classes create the token array for a corresponding FORMULA cell record. */
    virtual XclTokenArrayRef CreateCellTokenArray( const XclExpRoot& rRoot ) const = 0;
    /** Derived classes return true, if the own formula contains volatile functions. */
    virtual bool        IsVolatile() const = 0;

protected:
    /** Constructs the record with a single cell. */
    explicit            XclExpRangeFmlaBase(
                            sal_uInt16 nRecId, sal_uInt32 nRecSize, const ScAddress& rScPos );
    /** Constructs the record with a cell range. */
    explicit            XclExpRangeFmlaBase(
                            sal_uInt16 nRecId, sal_uInt32 nRecSize, const ScRange& rScRange );

    /** Extends the cell range to include the passed cell address. */
    void                Extend( const ScAddress& rScPos );

    /** Writes the range address covered by this record. */
    void                WriteRangeAddress( XclExpStream& rStrm ) const;

protected:
    XclRange            maXclRange;     /// Range described by this record.
    XclAddress          maBaseXclPos;   /// Address of base cell (first FORMULA record).
};

typedef std::shared_ptr< XclExpRangeFmlaBase > XclExpRangeFmlaRef;

// Array formulas =============================================================

class ScTokenArray;

/** Represents an ARRAY record that contains the token array of a matrix formula.

    An ARRAY record is stored following the first FORMULA record that is part
    of a matrix formula. All FORMULA records of a matrix formula contain a
    reference to the ARRAY record, while the ARRAY record contains the formula
    token array used by all formulas.
 */
class XclExpArray : public XclExpRangeFmlaBase
{
public:
    explicit            XclExpArray( XclTokenArrayRef xTokArr, const ScRange& rScRange );

    /** Creates and returns the token array for a corresponding FORMULA cell record. */
    virtual XclTokenArrayRef CreateCellTokenArray( const XclExpRoot& rRoot ) const override;
    /** Returns true, if the array formula contains volatile functions. */
    virtual bool        IsVolatile() const override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclTokenArrayRef    mxTokArr;       /// The token array of a matrix formula.
};

typedef std::shared_ptr< XclExpArray > XclExpArrayRef;

/** Caches all ARRAY records. */
class XclExpArrayBuffer : protected XclExpRoot
{
public:
    explicit            XclExpArrayBuffer( const XclExpRoot& rRoot );

    /** Inserts a new ARRAY record into the buffer and returns it. */
    XclExpArrayRef      CreateArray( const ScTokenArray& rScTokArr, const ScRange& rScRange );
    /** Tries to find an ARRAY record that corresponds to an ocMatRef token. */
    XclExpArrayRef FindArray( const ScTokenArray& rScTokArr, const ScAddress& rBasePos ) const;

private:
    typedef ::std::map< ScAddress, XclExpArrayRef > XclExpArrayMap;
    XclExpArrayMap      maRecMap;       /// Map containing the ARRAY records.
};

// Shared formulas ============================================================

/** Represents a SHRFMLA record that contains the token array of a shared formula.

    A SHRFMLA record is stored following the first FORMULA record that is part
    of a shared formula. All FORMULA records of a shared formula contain a
    reference to the SHRFMLA record, while the SHRFMLA record contains the
    formula token array used by all formulas.
 */
class XclExpShrfmla : public XclExpRangeFmlaBase
{
public:
    /** Creates a SHRFMLA record that consists of the passed cell address only. */
    explicit            XclExpShrfmla( XclTokenArrayRef xTokArr, const ScAddress& rScPos );

    /** Extends the cell range to include the passed cell address. */
    void                ExtendRange( const ScAddress& rScPos );

    /** Creates and returns the token array for a corresponding FORMULA cell record. */
    virtual XclTokenArrayRef CreateCellTokenArray( const XclExpRoot& rRoot ) const override;
    /** Returns true, if the shared formula contains volatile functions. */
    virtual bool        IsVolatile() const override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclTokenArrayRef    mxTokArr;       /// The token array of a shared formula.
    sal_uInt8           mnUsedCount;    /// Number of FORMULA records referring to this record.
};

typedef std::shared_ptr< XclExpShrfmla > XclExpShrfmlaRef;

/** Caches all SHRFMLA records and provides functions to update their ranges. */
class XclExpShrfmlaBuffer : protected XclExpRoot
{
public:
    explicit            XclExpShrfmlaBuffer( const XclExpRoot& rRoot );

    /** Tries to create a new or to update an existing SHRFMLA record.
        @return  An empty reference, if the passed token array does not contain
            a shared formula. If the token array is a shared formula, this
            function updates its cell range to include the passed cell position,
            if there is a SHRFMLA record for the passed token array; otherwise
            this function creates and returns a new SHRFMLA record. */
    XclExpShrfmlaRef CreateOrExtendShrfmla( const ScFormulaCell& rScCell, const ScAddress& rScPos );

private:
    /**
     * Check for presence of token that's not allowed in Excel's shared
     * formula. Refer to the "SharedParsedFormula" section of [MS-XLS] spec
     * for more info.
     */
    bool IsValidTokenArray( const ScTokenArray& rArray ) const;

    typedef std::unordered_map<const ScTokenArray*, XclExpShrfmlaRef> TokensType;
    typedef std::unordered_set<const ScTokenArray*> BadTokenArraysType;

    TokensType         maRecMap;    /// Map containing the SHRFMLA records.
    BadTokenArraysType maBadTokens; /// shared tokens we should *not* export as SHRFMLA
};

// Multiple operations ========================================================

struct XclMultipleOpRefs;

/** Represents a TABLEOP record for a multiple operations range. */
class XclExpTableop : public XclExpRangeFmlaBase
{
public:
    explicit            XclExpTableop( const ScAddress& rScPos,
                            const XclMultipleOpRefs& rRefs, sal_uInt8 nScMode );

    /** Returns true, if the cell range has been extended to the passed position.
        @descr  All references passed in rRefs must fit the ranges passed in the constructor. */
    bool                TryExtend( const ScAddress& rScPos, const XclMultipleOpRefs& rRefs );

    /** Finalizes the record. Tests on valid cell range and reference addresses. */
    void                Finalize();

    /** Creates and returns the token array for a corresponding FORMULA cell record. */
    virtual XclTokenArrayRef CreateCellTokenArray( const XclExpRoot& rRoot ) const override;
    /** Returns true, if the multiple operations range is volatile. */
    virtual bool        IsVolatile() const override;
    /** Writes the record if it is valid. */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    /** Returns true, if the passed cell position can be appended to this record. */
    bool                IsAppendable( sal_uInt16 nXclCol, sal_uInt16 nXclRow ) const;

    /** Writes the contents of the TABLEOP record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    sal_uInt16          mnLastAppXclCol;/// Column index of last appended cell.
    sal_uInt16          mnColInpXclCol; /// Column index of column input cell.
    sal_uInt32          mnColInpXclRow; /// Row index of column input cell.
    sal_uInt16          mnRowInpXclCol; /// Column index of row input cell.
    sal_uInt32          mnRowInpXclRow; /// Row index of row input cell.
    sal_uInt8           mnScMode;       /// Type of the multiple operation (Calc constant).
    bool                mbValid;        /// true = Contains valid references.
};

typedef std::shared_ptr< XclExpTableop > XclExpTableopRef;

/** Contains all created TABLEOP records and supports creating or updating them. */
class XclExpTableopBuffer : protected XclExpRoot
{
public:
    explicit            XclExpTableopBuffer( const XclExpRoot& rRoot );

    /** Tries to update an existing or to create a new TABLEOP record.
        @return  Reference to the TABLEOP record for this cell (existing or new),
            or an empty reference, if rScTokArr does not contain a multiple
            operations formula. */
    XclExpTableopRef    CreateOrExtendTableop(
                            const ScTokenArray& rScTokArr, const ScAddress& rScPos );

    /** Finalizes all contained TABLEOP records. */
    void                Finalize();

private:
    /** Tries to create a new TABLEOP record, if rRefs contains valid references. */
    XclExpTableopRef    TryCreate( const ScAddress& rScPos, const XclMultipleOpRefs& rRefs );

private:
    typedef XclExpRecordList< XclExpTableop > XclExpTableopList;
    XclExpTableopList   maTableopList;  /// List of all TABLEOP records.
};

// Cell records

/** The base class of all cell records. */
class XclExpCellBase : public XclExpRecord
{
public:
    /** Returns the (first) address of the cell(s). */
    inline const XclAddress& GetXclPos() const { return maXclPos; }
    /** Returns the (first) Excel column index of the cell(s). */
    inline sal_uInt16   GetXclCol() const { return maXclPos.mnCol; }
    /** Returns the Excel row index of the cell. */
    inline sal_uInt32   GetXclRow() const { return maXclPos.mnRow; }

    /** Derived classes return the column index of the last contained cell. */
    virtual sal_uInt16  GetLastXclCol() const = 0;
    /** Derived classes return the XF identifier of the first contained cell. */
    virtual sal_uInt32  GetFirstXFId() const = 0;
    /** Derived classes return true, if this record does not contain at least one valid cell. */
    virtual bool        IsEmpty() const = 0;
    /** Derived classes return whether the cell contains multi-line text. */
    virtual bool        IsMultiLineText() const;

    /** Derived classes try to merge the contents of the passed cell to own data. */
    virtual bool        TryMerge( const XclExpCellBase& rCell );
    /** Derived classes convert the XF identifier(s) into the Excel XF index(es).
        @param rXFIndexes  The converted XF index(es) are inserted here. */
    virtual void        ConvertXFIndexes( const XclExpRoot& rRoot ) = 0;
    /** Derived classes for blank cells insert the Excel XF index(es) into the passed vector. */
    virtual void        GetBlankXFIndexes( ScfUInt16Vec& rXFIndexes ) const;
    /** Derived classes for blank cells remove unused Excel XF index(es). */
    virtual void        RemoveUnusedBlankCells( const ScfUInt16Vec& rXFIndexes );

protected:
    explicit            XclExpCellBase(
                            sal_uInt16 nRecId, sal_Size nContSize, const XclAddress& rXclPos );

    /** Sets this record to a new column position. */
    inline void         SetXclCol( sal_uInt16 nXclCol ) { maXclPos.mnCol = nXclCol; }

private:
    XclAddress          maXclPos;       /// Address of the cell.
};

typedef std::shared_ptr< XclExpCellBase > XclExpCellRef;

// Single cell records ========================================================

/** Base class for all cell records not supporting multiple contents. */
class XclExpSingleCellBase : public XclExpCellBase
{
public:
    /** Returns the last column, which is equal to the first column for single cells. */
    virtual sal_uInt16  GetLastXclCol() const override;
    /** Return the XF identifier of the cell. */
    virtual sal_uInt32  GetFirstXFId() const override;
    /** Returns true, if this record does not contain at least one valid cell. */
    virtual bool        IsEmpty() const override;
    /** Converts the XF identifier into the Excel XF index. */
    virtual void        ConvertXFIndexes( const XclExpRoot& rRoot ) override;
    /** Writes cell address, XF index, and calls WriteContents() for each cell. */
    virtual void        Save( XclExpStream& rStrm ) override;

protected:
    explicit            XclExpSingleCellBase( sal_uInt16 nRecId, sal_Size nContSize,
                            const XclAddress& rXclPos, sal_uInt32 nXFId );

    explicit            XclExpSingleCellBase( const XclExpRoot& rRoot,
                            sal_uInt16 nRecId, sal_Size nContSize, const XclAddress& rXclPos,
                            const ScPatternAttr* pPattern, sal_Int16 nScript, sal_uInt32 nForcedXFId );

    inline void         SetContSize( sal_Size nContSize ) { mnContSize = nContSize; }
    inline sal_Size     GetContSize() const { return mnContSize; }

    inline void         SetXFId( sal_uInt32 nXFId ) { maXFId.mnXFId = nXFId; }
    inline sal_uInt32   GetXFId() const { return maXFId.mnXFId; }

private:
    /** Writes cell address, XF index, and calls WriteContents() for each cell. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;
    /** Derived classes write the contents of the specified cell (without XF index). */
    virtual void        WriteContents( XclExpStream& rStrm ) = 0;

private:
    XclExpXFId          maXFId;         /// The XF identifier of the cell formatting.
    sal_Size            mnContSize;     /// The size of the cell contents.
};

/** Represents a NUMBER record that describes a cell with a double value. */
class XclExpNumberCell : public XclExpSingleCellBase
{
    DECL_FIXEDMEMPOOL_NEWDEL( XclExpNumberCell )

public:
    explicit            XclExpNumberCell( const XclExpRoot& rRoot, const XclAddress& rXclPos,
                            const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId,
                            double fValue );

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
private:
    virtual void        WriteContents( XclExpStream& rStrm ) override;

private:
    double              mfValue;        /// The cell value.
};

/** Represents a BOOLERR record that describes a cell with a Boolean value. */
class XclExpBooleanCell : public XclExpSingleCellBase
{
    DECL_FIXEDMEMPOOL_NEWDEL( XclExpBooleanCell )

public:
    explicit            XclExpBooleanCell( const XclExpRoot& rRoot, const XclAddress& rXclPos,
                            const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId,
                            bool bValue );

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
private:
    virtual void        WriteContents( XclExpStream& rStrm ) override;

private:
    bool                mbValue;        /// The cell value.
};

class XclExpHyperlinkHelper;
class EditTextObject;

/** Represents a text cell record.

    May contain a BIFF2-BIFF7 LABEL record for a simple string, or a BIFF2-BIFF7
    RSTRING record for a formatted string, or a BIFF8 LABELSST string for any
    string (simply stores a reference to the Shared String Table).
 */
class XclExpLabelCell : public XclExpSingleCellBase
{
    DECL_FIXEDMEMPOOL_NEWDEL( XclExpLabelCell )

public:
    /** Constructs the record from an unformatted Calc string cell. */
    explicit            XclExpLabelCell( const XclExpRoot& rRoot, const XclAddress& rXclPos,
                            const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId,
                            const OUString& rStr );

    /** Constructs the record from a formatted Calc edit cell. */
    explicit            XclExpLabelCell( const XclExpRoot& rRoot, const XclAddress& rXclPos,
                            const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId,
                            const EditTextObject* pEditText, XclExpHyperlinkHelper& rHlinkHelper );

    /** Returns true if the cell contains multi-line text. */
    virtual bool        IsMultiLineText() const override;

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
private:
    /** Initializes the record contents. Called from constructors. */
    void                Init( const XclExpRoot& rRoot,
                            const ScPatternAttr* pPattern, XclExpStringRef xText );

    virtual void        WriteContents( XclExpStream& rStrm ) override;

private:
    XclExpStringRef     mxText;         /// The cell text.
    sal_uInt32          mnSstIndex;     /// Index into Shared String Table (only used for BIFF8).
    bool                mbLineBreak;    /// True = cell has automatic linebreaks enabled.
};

class ScFormulaCell;

/** Represents a FORMULA record that describes a cell with a formula. */
class XclExpFormulaCell : public XclExpSingleCellBase
{
    DECL_FIXEDMEMPOOL_NEWDEL( XclExpFormulaCell )

public:
    explicit            XclExpFormulaCell( const XclExpRoot& rRoot, const XclAddress& rXclPos,
                            const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId,
                            const ScFormulaCell& rScFmlaCell,
                            XclExpArrayBuffer& rArrayBfr,
                            XclExpShrfmlaBuffer& rShrfmlaBfr,
                            XclExpTableopBuffer& rTableopBfr );

    /** Writes the FORMULA record and additional records related to the formula. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    virtual void        WriteContents( XclExpStream& rStrm ) override;

private:
    ScFormulaCell&      mrScFmlaCell;   /// The Calc formula cell.
    XclTokenArrayRef    mxTokArr;       /// The token array of the formula.
    XclExpRangeFmlaRef  mxAddRec;       /// Additional record for matrix/shared formulas.
    XclExpRecordRef     mxStringRec;    /// STRING record for string result.
};

// Multiple cell records ======================================================

struct XclExpMultiXFId : public XclExpXFId
{
    sal_uInt16          mnCount;        /// Number of XF identifiers.

    inline explicit     XclExpMultiXFId( sal_uInt32 nXFId, sal_uInt16 nCount = 1 ) :
                            XclExpXFId( nXFId ), mnCount( nCount ) {}
};

/** Base class for all cell records supporting multiple contents. */
class XclExpMultiCellBase : public XclExpCellBase
{
public:
    /** Returns the column index of the last cell this record describes. */
    virtual sal_uInt16  GetLastXclCol() const override;
    /** Return the XF identifier of the first contained cell. */
    virtual sal_uInt32  GetFirstXFId() const override;
    /** Returns true, if this record does not contain at least one valid cell. */
    virtual bool        IsEmpty() const override;

    /** Convert all XF identifiers into the Excel XF indexes. */
    virtual void        ConvertXFIndexes( const XclExpRoot& rRoot ) override;
    /** Writes the record, calls WriteContents() for each contained cell.
        @descr  May write several records, if unused XF indexes are contained. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

protected:
    explicit            XclExpMultiCellBase( sal_uInt16 nRecId, sal_uInt16 nMulRecId,
                            sal_Size nContSize, const XclAddress& rXclPos );

    /** Returns the number of cells this record represents. */
    sal_uInt16          GetCellCount() const;

    /** Appends the passed XF identifier nCount times to the list of XF identifiers. */
    void                AppendXFId( const XclExpMultiXFId& rXFId );
    /** Appends the passed cell format nCount times to the list of XF identifiers. */
    void                AppendXFId( const XclExpRoot& rRoot,
                            const ScPatternAttr* pPattern, sal_uInt16 nScript,
                            sal_uInt32 nForcedXFId, sal_uInt16 nCount = 1 );

    /** Tries to merge the XF ID list of the passed cell with the own list. */
    bool                TryMergeXFIds( const XclExpMultiCellBase& rCell );
    /** Inserts the Excel XF index(es) into the passed vector. */
    void                GetXFIndexes( ScfUInt16Vec& rXFIndexes ) const;

    /** Removes unused Excel XF index(es).
        @param rXFIndexes  Specifies which XF indexes are used. */
    void                RemoveUnusedXFIndexes( const ScfUInt16Vec& rXFIndexes );

private:
    /** Derived classes write the remaining contents of the specified cell (without XF index).
        @param nRelCol  Relative column index (starts with 0 for first cell of this record). */
    virtual void        WriteContents( XclExpStream& rStrm, sal_uInt16 nRelCol ) = 0;
    virtual void        WriteXmlContents( XclExpXmlStream& rStrm, const XclAddress& rAddress, sal_uInt32 nXFId, sal_uInt16 nRelCol ) = 0;

private:
    typedef ::std::vector< XclExpMultiXFId > XclExpMultiXFIdDeq;

    sal_uInt16          mnMulRecId;     /// Record ID for multiple record variant.
    sal_Size            mnContSize;     /// Data size of contents for one cell
    XclExpMultiXFIdDeq  maXFIds;        /// The XF identifiers of the cell formatting.
};

/** Represents a BLANK or MULBLANK record that describes empty but formatted cells. */
class XclExpBlankCell : public XclExpMultiCellBase
{
    DECL_FIXEDMEMPOOL_NEWDEL( XclExpBlankCell )

public:
    explicit            XclExpBlankCell( const XclAddress& rXclPos, const XclExpMultiXFId& rXFId );

    explicit            XclExpBlankCell( const XclExpRoot& rRoot,
                            const XclAddress& rXclPos, sal_uInt16 nLastXclCol,
                            const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId );

    /** Tries to merge the contents of the passed cell to own data. */
    virtual bool        TryMerge( const XclExpCellBase& rCell ) override;
    /** Inserts the Excel XF index(es) into the passed vector. */
    virtual void        GetBlankXFIndexes( ScfUInt16Vec& rXFIndexes ) const override;
    /** Tries to remove unused Excel XF index(es). */
    virtual void        RemoveUnusedBlankCells( const ScfUInt16Vec& rXFIndexes ) override;

private:
    /** Writes the remaining contents of the specified cell (without XF index). */
    virtual void        WriteContents( XclExpStream& rStrm, sal_uInt16 nRelCol ) override;
    virtual void        WriteXmlContents( XclExpXmlStream& rStrm, const XclAddress& rAddress, sal_uInt32 nXFId, sal_uInt16 nRelCol ) override;
};

/** Represents an RK or MULRK record that describes cells with a compressed double values. */
class XclExpRkCell : public XclExpMultiCellBase
{
    DECL_FIXEDMEMPOOL_NEWDEL( XclExpRkCell )

public:
    explicit            XclExpRkCell( const XclExpRoot& rRoot, const XclAddress& rXclPos,
                            const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId,
                            sal_Int32 nRkValue );

    /** Tries to merge the contents of the passed cell to own data. */
    virtual bool        TryMerge( const XclExpCellBase& rCell ) override;

private:
    /** Writes the remaining contents of the specified cell (without XF index). */
    virtual void        WriteContents( XclExpStream& rStrm, sal_uInt16 nRelCol ) override;
    virtual void        WriteXmlContents( XclExpXmlStream& rStrm, const XclAddress& rAddress, sal_uInt32 nXFId, sal_uInt16 nRelCol ) override;

private:
    ScfInt32Vec         maRkValues;     /// The cell values.
};

// Rows and Columns

class ScOutlineArray;

/** Base class for buffers containing row or column outline data. */
class XclExpOutlineBuffer
{
public:
    /** Returns true, if a collapsed group ends at the last processed position. */
    inline bool         IsCollapsed() const { return mbCurrCollapse; }
    /** Returns the highest level of an open group at the last processed position. */
    inline sal_uInt8    GetLevel() const { return ::std::min( mnCurrLevel, EXC_OUTLINE_MAX ); }

protected:
    /** Constructs the outline buffer.
        @param bRows  true = Process row outline array; false = Process column outline array. */
    explicit            XclExpOutlineBuffer( const XclExpRoot& rRoot, bool bRows );

    /** Updates the current state by processing the settings at the passed Calc position. */
    void                UpdateColRow( SCCOLROW nScPos );

private:
    /** Data about an outline level. */
    struct XclExpLevelInfo
    {
        SCCOLROW            mnScEndPos;         /// The end position of a group in a level.
        bool                mbHidden;           /// true = Group in this level is hidden.
        inline explicit     XclExpLevelInfo() : mnScEndPos( 0 ), mbHidden( false ) {}
    };
    typedef ::std::vector< XclExpLevelInfo > XclExpLevelInfoVec;

    const ScOutlineArray* mpScOLArray;      /// Pointer to Calc outline array.
    XclExpLevelInfoVec  maLevelInfos;       /// Info for current row and all levels.
    sal_uInt8           mnCurrLevel;        /// Highest level of an open group for current position.
    bool                mbCurrCollapse;     /// true = Collapsed group ends at current position.
};

/** The outline buffer for column outlines. */
class XclExpColOutlineBuffer : public XclExpOutlineBuffer
{
public:
    inline explicit     XclExpColOutlineBuffer( const XclExpRoot& rRoot ) :
                            XclExpOutlineBuffer( rRoot, false ) {}

    /** Updates the current state by processing the settings of the passed Calc column. */
    inline void         Update( SCCOL nScCol )
                            { UpdateColRow( static_cast< SCCOLROW >( nScCol ) ); }
};

/** The outline buffer for row outlines. */
class XclExpRowOutlineBuffer : public XclExpOutlineBuffer
{
public:
    inline explicit     XclExpRowOutlineBuffer( const XclExpRoot& rRoot ) :
                            XclExpOutlineBuffer( rRoot, true ) {}

    /** Updates the current state by processing the settings of the passed Calc row. */
    inline void         Update( SCROW nScRow )
                            { UpdateColRow( static_cast< SCCOLROW >( nScRow ) ); }
};

/** Represents a GUTS record containing the level count of row and column outlines. */
class XclExpGuts : public XclExpRecord
{
public:
    explicit            XclExpGuts( const XclExpRoot& rRoot );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    sal_uInt16          mnColLevels;    /// Number of visible column outline levels.
    sal_uInt16          mnColWidth;     /// Width of column outline area (pixels).
    sal_uInt16          mnRowLevels;    /// Number of visible row outline levels.
    sal_uInt16          mnRowWidth;     /// Width of row outline area (pixels).
};

/** Represents a DIMENSIONS record containing the used area of a sheet. */
class XclExpDimensions : public XclExpRecord
{
public:
    explicit            XclExpDimensions( const XclExpRoot& rRoot );

    /** Sets the used area to the record. */
    void                SetDimensions(
                            sal_uInt16 nFirstUsedXclCol, sal_uInt32 nFirstUsedXclRow,
                            sal_uInt16 nFirstFreeXclCol, sal_uInt32 nFirstFreeXclRow );

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
private:
    /** Writes the contents of the DIMENSIONS record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    sal_uInt32          mnFirstUsedXclRow;  /// First used row.
    sal_uInt32          mnFirstFreeXclRow;  /// First unused row after used area.
    sal_uInt16          mnFirstUsedXclCol;  /// First used column.
    sal_uInt16          mnFirstFreeXclCol;  /// First free column after used area.
};

/** Represents the DEFCOLWIDTH record containing the default column width of a sheet.

    Excel stores the default column width in entire character widths of the '0'
    character using the application default font (i.e. the default width is 10,
    if the '0' character fits 10 times into a cell in a column with default
    width.

    Half of character width is reserved for non character display.
    It is margin padding (two on each side) and padding for the gridlines.

    The IsDefWidth() function returns true, if the passed width (measured in
    1/256 of the width of the '0' character) could be converted exactly to the
    default width. If the passed width is rounded up or down to get the default
    width, the function returns false.
 */
class XclExpDefcolwidth : public XclExpUInt16Record, protected XclExpRoot
{
public:
    explicit            XclExpDefcolwidth( const XclExpRoot& rRoot );

    /** Returns true, if the own default width exactly matches the passed width. */
    bool                IsDefWidth( sal_uInt16 nXclColWidth ) const;

    /** Sets the passed column width (in 1/256 character width) as default width. */
    void                SetDefWidth( sal_uInt16 nXclColWidth );
};

/** Contains the column settings for a range of columns.

    After construction the record contains a temporary XF identifier returned
    from the XF buffer. After creating the entire Excel document in memory, the
    ConvertXFIndexes() function converts it into the real Excel XF index.
 */
class XclExpColinfo : public XclExpRecord, protected XclExpRoot
{
public:
    /** Constructs the record with the settings in the Calc document. */
    explicit            XclExpColinfo( const XclExpRoot& rRoot,
                            SCCOL nScCol, SCROW nLastScRow,
                            XclExpColOutlineBuffer& rOutlineBfr );

    /** Converts the XF identifier into the Excel XF index, returns the latter. */
    sal_uInt16          ConvertXFIndexes();

    /** Tries to merge this record with the passed record.
        @descr  Possible, if passed record directly follows this record and has equal contents.
        @return  true = This record is equal to passed record and has been updated. */
    bool                TryMerge( const XclExpColinfo& rColInfo );

    /** Returns the Excel width of the column(s). */
    inline sal_uInt16   GetColWidth() const { return mnWidth; }
    /** Returns the final Excel XF index of the column(s). */
    inline sal_uInt16   GetXFIndex() const { return maXFId.mnXFIndex; }
    /** Returns the number of columns represented by this record. */
    inline sal_uInt16   GetColCount() const { return mnLastXclCol - mnFirstXclCol + 1; }

    /** Returns true, if the column has default format and width. */
    bool                IsDefault( const XclExpDefcolwidth& rDefColWidth ) const;

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the contents of this COLINFO record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclExpXFId          maXFId;             /// The XF identifier for column default format.
    bool                mbCustomWidth;      /// True = Column width is different from default width
    sal_uInt16          mnWidth;            /// Excel width of the column.
    sal_uInt16          mnScWidth;          /// Calc width of the column.
    sal_uInt16          mnFlags;            /// Additional column flags.
    sal_uInt16          mnFirstXclCol;      /// Index to first column.
    sal_uInt16          mnLastXclCol;       /// Index to last column.
};

/** Contains COLINFO records for all columns of a Calc sheet.

    On construction one COLINFO record per column is created. After creating
    the entire Excel document in memory, the ConvertXFIndexes() function converts
    all temporary XF identifiers into real Excel XF indexes and merges all equal
    COLINFO records together.
 */
class XclExpColinfoBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpColinfoBuffer( const XclExpRoot& rRoot );

    /** Initializes the buffer: finds settings and formatting of all columns.
        @param nLastScRow  Last row used to find default formatting. */
    void                Initialize( SCROW nLastScRow );
    /** Converts the XF identifiers into the Excel XF indexes and merges equal columns.
        @param rXFIndexes  Returns the final XF indexes of all columns. */
    void                Finalize( ScfUInt16Vec& rXFIndexes );

    /** Writes all COLINFO records of this buffer. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpColinfo >   XclExpColinfoList;
    typedef XclExpColinfoList::RecordRefType    XclExpColinfoRef;

    XclExpColinfoList   maColInfos;         /// List of COLINFO records.
    XclExpDefcolwidth   maDefcolwidth;      /// The DEFCOLWIDTH record.
    XclExpColOutlineBuffer maOutlineBfr;    /// Buffer for column outline groups.
};

class XclExpRow;

/** Contains all possible default row settings. */
struct XclExpDefaultRowData
{
    sal_uInt16          mnFlags;            /// Default flags for unspecified rows.
    sal_uInt16          mnHeight;           /// Default height for unspecified rows.

    explicit            XclExpDefaultRowData();
    explicit            XclExpDefaultRowData( const XclExpRow& rRow );

    /** Returns true, if rows are hidden by default. */
    inline bool         IsHidden() const { return ::get_flag( mnFlags, EXC_DEFROW_HIDDEN ); }
    /** Returns true, if the rows have a manually set height by default. */
    inline bool         IsUnsynced() const { return ::get_flag( mnFlags, EXC_DEFROW_UNSYNCED ); }
};

/** Represents a DEFROWHEIGHT record containing default format for unused rows. */
class XclExpDefrowheight : public XclExpRecord
{
public:
    explicit            XclExpDefrowheight();

    /** Sets the passed default data as current record contents. */
    void                SetDefaultData( const XclExpDefaultRowData& rDefData );
    XclExpDefaultRowData& GetDefaultData() { return maDefData; }
private:
    /** Writes the contents of the record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclExpDefaultRowData maDefData;         /// Record data.
};

/** Represents a ROW record and additionally contains all cells records of a row.

    This class contains all cell records of a row in a spreadsheet. There are 2
    cell records in Excel that support storing a range of cells in one record
    (MULBLANK for multiple blank cells, and MULRK for multiple RK values). The
    insertion functions try to merge a new inserted cell with existing
    neighbors, if this is supported by the current type of cell record.

    The Finalize() function converts the XF identifiers of all cell records to
    the final Excel XF indexes. Then a default
 */
class XclExpRow : public XclExpRecord, protected XclExpRoot
{
public:
    /** Constructs the ROW record and converts the Calc row settings.
        @param bAlwaysEmpty  true = This row will not be filled with blank cells
            in the Finalize() function. */
    explicit            XclExpRow( const XclExpRoot& rRoot, sal_uInt32 nXclRow,
                            XclExpRowOutlineBuffer& rOutlineBfr, bool bAlwaysEmpty );

    /** Returns the excel row index of this ROW record. */
    inline sal_uInt32   GetXclRow() const { return mnXclRow; }
    /** Returns the height of the row in twips. */
    inline sal_uInt16   GetHeight() const { return mnHeight; }
    /** Returns true, if this row does not contain at least one valid cell. */
    inline bool         IsEmpty() const { return maCellList.IsEmpty(); }
    /** Returns true, if this row is hidden. */
    inline bool         IsHidden() const { return ::get_flag( mnFlags, EXC_ROW_HIDDEN ); }
    /** Returns true, if this row contains a manually set height. */
    inline bool         IsUnsynced() const { return ::get_flag( mnFlags, EXC_ROW_UNSYNCED ); }
    /** Returns true, if this row is enabled (will be exported). */
    inline bool         IsEnabled() const { return mbEnabled; }

    /** Appends the passed cell object to this row. */
    void                AppendCell( XclExpCellRef xCell, bool bIsMergedBase );

    /** Converts all XF identifiers into the Excel XF indexes. */
    void                Finalize( const ScfUInt16Vec& rColXFIndexes,
                                  bool bUpdateProgress );

    /** Returns the column index of the first used cell in this row.
        @descr  This function can only be called after Finalize(). */
    sal_uInt16          GetFirstUsedXclCol() const;
    /** Returns the column index of the first unused cell following all used cells in this row.
        @descr  This function can only be called after Finalize(). */
    sal_uInt16          GetFirstFreeXclCol() const;

    /** Returns true, if this row may be omitted by using the DEFROWHEIGHT record.
        @descr  A row may be omitted, if it does not contain any cell or
        explicit default cell formatting, and is not part of an outline.
        This function can only be called after Finalize(). */
    bool                IsDefaultable() const;
    /** Disables this row, if it is defaultable and has the passed default format.
        @descr  Disabled rows will not be saved.
            This function can only be called after Finalize(). */
    void                DisableIfDefault( const XclExpDefaultRowData& rDefRowData );

    /** Writes all cell records of this row. */
    void                WriteCellList( XclExpStream& rStrm );

    /** Writes the ROW record if the row is not disabled (see DisableIfDefault() function). */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

    inline sal_uInt32   GetXclRowRpt() const { return mnXclRowRpt; }
    inline void         SetXclRowRpt( sal_uInt32 nRpt ){ mnXclRowRpt = nRpt; }
private:
    /** Inserts a cell at the specified list position, tries to merge with neighbors. */
    void                InsertCell( XclExpCellRef xCell, size_t nPos, bool bIsMergedBase );

    /** Writes the contents of the ROW record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpCellBase > XclExpCellList;

    XclExpCellList      maCellList;         /// List of cell records for this row.
    sal_uInt32          mnXclRow;           /// Excel row index of this row.
    sal_uInt16          mnHeight;           /// Row height in twips.
    sal_uInt16          mnFlags;            /// Flags for the ROW record.
    sal_uInt16          mnXFIndex;          /// Default row formatting.
    sal_uInt16          mnOutlineLevel;     /// Outline Level (for OOXML)
    sal_uInt32          mnXclRowRpt;
    sal_uInt32          mnCurrentRow;
    bool                mbAlwaysEmpty;      /// true = Do not add blank cells in Finalize().
    bool                mbEnabled;          /// true = Write this ROW record.
};

/** Collects all rows which contain all cells of a sheet.

    This row buffer automatically creates ROW records when cells are inserted
    with the AppendCell() function. It is possible to force creation of more
    ROW records with the CreateRows() function. In both cases, all preceding
    missing ROW records are inserted too.
 */
class XclExpRowBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpRowBuffer( const XclExpRoot& rRoot );

    /** Appends the passed cell object to the row that the cell specifies. */
    void                AppendCell( XclExpCellRef xCell, bool bIsMergedBase );
    /** Forces insertion of all ROW records before the passed row. */
    void                CreateRows( SCROW nFirstFreeScRow );

    /** Converts all XF identifiers into the Excel XF indexes and calculates default formats.
        @param rDefRowData  (out-param) The default row format is returned here.
        @param rColXFIndexes  The column default XF indexes. */
    void                Finalize( XclExpDefaultRowData& rDefRowData, const ScfUInt16Vec& rColXFIndexes );

    /** Writes the DIMENSIONS record, all ROW records and all cell records. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

    XclExpDimensions&   GetDimensions() { return maDimensions;}

private:
    /** Returns access to the specified ROW record. Inserts preceding missing ROW records.
        @param bRowAlwaysEmpty  true = Created rows will not be filled with blank cells
            in the XclExpRow::Finalize() function. */
    XclExpRow&          GetOrCreateRow( sal_uInt32 nXclRow, bool bRowAlwaysEmpty );

private:
    typedef std::shared_ptr<XclExpRow>  RowRef;
    typedef ::std::map<sal_uInt32, RowRef>  RowMap;

    RowMap              maRowMap;
    XclExpRowOutlineBuffer maOutlineBfr;    /// Buffer for row outline groups.
    XclExpDimensions    maDimensions;       /// DIMENSIONS record for used area.
};

// Cell Table

class XclExpNote;
class XclExpMergedcells;
class XclExpHyperlink;
class XclExpDval;

/** This class contains the cell contents and more of an entire sheet.

    The cell table includes the settings and default formatting of all columns,
    the settings and default formatting of all used rows, and the contents of
    all cells of one sheet in a spreadsheet document.

    The constructor does all the work creating the cell table. It reads the
    Calc sheet and converts all columns, rows, and cells to Excel record data.
    Additionally, hyperlink records, note records, additional records for
    formula cells, data validation records, and outline records are created.

    The Finalize() function does even more work. It calculates default column
    settings and removes column records that are equal to this default. The
    same happens with rows: A default format is calculated for each row, and
    all blank cells in this row that have the same format are removed. Then,
    the most used row settings are calculated, and all empty rows that have the
    same settings are removed too.

    Records that are not stored inside the cell table area in an Excel file
    (i.e. DEFROWHEIGHT record, NOTE records, MERGEDCELLS record, HLINK records,
    DVAL and DV records for data validation) can be accessed with the function
    CreateRecord(). It returns the reference to the respective record (or
    record list) which can be inserted into a record list.
 */
class XclExpCellTable : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpCellTable( const XclExpRoot& rRoot );

    /** Converts all XF identifiers into the Excel XF indexes and calculates default formats. */
    void                Finalize();

    /** Returns the reference to an internal record specified by the passed record id.
        @param nRecId  The record identifier that specifies which record is
            returned. Possible values are: EXC_ID_DEFROWHEIGHT, EXC_ID_NOTE,
            EXC_ID_MERGEDCELLS, EXC_ID_HLINK, EXC_ID_DVAL. */
    XclExpRecordRef     CreateRecord( sal_uInt16 nRecId ) const;
    /** Saves the entire cell table. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpNote >      XclExpNoteList;
    typedef XclExpRecordList< XclExpHyperlink > XclExpHyperlinkList;

    typedef std::shared_ptr< XclExpDefrowheight >        XclExpDefrowhRef;
    typedef std::shared_ptr< XclExpNoteList >            XclExpNoteListRef;
    typedef std::shared_ptr< XclExpMergedcells >         XclExpMergedcellsRef;
    typedef std::shared_ptr< XclExpHyperlinkList >       XclExpHyperlinkRef;
    typedef std::shared_ptr< XclExpDval >                XclExpDvalRef;
    typedef std::shared_ptr< XclExtLst >                 XclExtLstRef;

    XclExpColinfoBuffer maColInfoBfr;       /// Buffer for column formatting.
    XclExpRowBuffer     maRowBfr;           /// Rows and cell records.
    XclExpArrayBuffer   maArrayBfr;         /// Buffer for ARRAY records.
    XclExpShrfmlaBuffer maShrfmlaBfr;       /// Buffer for SHRFMLA records.
    XclExpTableopBuffer maTableopBfr;       /// Buffer for TABLEOP records.
    XclExpDefrowhRef    mxDefrowheight;     /// DEFROWHEIGHT record for default row format.
    XclExpRecordRef     mxGuts;             /// GUTS record for outline areas.
    XclExpNoteListRef   mxNoteList;         /// List of NOTE records.
    XclExpMergedcellsRef mxMergedcells;     /// MERGEDCELLS record for merged cell ranges.
    XclExpHyperlinkRef  mxHyperlinkList;    /// List of HLINK records.
    XclExpDvalRef       mxDval;             /// Data validation with DVAL and DV records.
    XclExtLstRef        mxExtLst;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
