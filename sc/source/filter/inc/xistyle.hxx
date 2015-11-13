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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XISTYLE_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XISTYLE_HXX

#include <list>
#include <memory>
#include <vector>
#include <tools/mempool.hxx>
#include <boost/noncopyable.hpp>
#include "rangelst.hxx"
#include "patattr.hxx"
#include "xladdress.hxx"
#include "xlstyle.hxx"
#include "xiroot.hxx"

struct ScAttrEntry;
enum class SvxBoxItemLine;

/* ============================================================================
- Buffers for style records (PALETTE, FONT, FORMAT, XF)
    and a container for XF indexes for every used cell in a sheet.
============================================================================ */

// PALETTE record - color information =========================================

/** Stores the default colors for the current BIFF version and the contents of
    a PALETTE record. */
class XclImpPalette : public XclDefaultPalette
{
public:
    explicit            XclImpPalette( const XclImpRoot& rRoot );

    /** Clears all buffered data, used to set up for a new sheet. */
    void                Initialize();

    /** Returns the RGB color data for a (non-zero-based) Excel palette entry.
        @descr  First looks for a color read from file, then looks for a default color.
        @return  The color from current or default palette or COL_AUTO, if nothing else found. */
    ColorData           GetColorData( sal_uInt16 nXclIndex ) const;
    /** Returns the color for a (non-zero-based) Excel palette entry.
        @descr  First looks for a color read from file, then looks for a default color.
        @return  The color from current or default palette or COL_AUTO, if nothing else found. */
    inline Color        GetColor( sal_uInt16 nXclIndex ) const
                            { return Color( GetColorData( nXclIndex ) ); }

    /** Reads a PALETTE record. */
    void                ReadPalette( XclImpStream& rStrm );

private:
    void ExportPalette();
    typedef ::std::vector< ColorData > ColorDataVec;
    ColorDataVec        maColorTable;       /// Colors read from file.
    const XclImpRoot&             mrRoot;
};

// FONT record - font information =============================================

/** Stores all data of an Excel font and provides import of FONT records. */
class XclImpFont : protected XclImpRoot
{
public:
    explicit            XclImpFont( const XclImpRoot& rRoot );

    /** Constructs a font from font data.
        @descr  Special handling for font style (bold, italic) in font name,
        overwrites settings in rFontData. */
    explicit            XclImpFont( const XclImpRoot& rRoot, const XclFontData& rFontData );

    /** Sets all font attributes to used or unused. */
    void                SetAllUsedFlags( bool bUsed );
    /** Sets the passed font data and all used flags to 'used'. */
    void                SetFontData( const XclFontData& rFontData, bool bHasCharSet );

    /** Returns read-only access to font data. */
    inline const XclFontData& GetFontData() const { return maData; }
    /** Returns true, if the font character set is valid. */
    inline bool         HasCharSet() const { return mbHasCharSet; }
    /** Returns true, if the font contains superscript or subscript. */
    inline bool         HasEscapement() const { return maData.mnEscapem != EXC_FONTESC_NONE; }
    /** Returns the text encoding for strings used with this font. */
    rtl_TextEncoding    GetFontEncoding() const;

    /** Returns true, if this font contains characters for Asian scripts (CJK). */
    inline bool         HasAsianChars() const { return mbHasAsian; }

    /** Reads a FONT record for all BIFF versions. */
    void                ReadFont( XclImpStream& rStrm );
    /** Reads an EFONT record (BIFF2 font color). */
    void                ReadEfont( XclImpStream& rStrm );
    /** Reads the font block from a CF (conditional format) record. */
    void                ReadCFFontBlock( XclImpStream& rStrm );

    /** Fills all font properties to the item set.
        @param rItemSet  The destination item set.
        @param eType  The type of Which-IDs.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                FillToItemSet( SfxItemSet& rItemSet, XclFontItemType eType,
                            bool bSkipPoolDefs = false ) const;
    /** Writes all font properties to the passed property set.
        @param pFontColor  If set, overrides internal stored font color. */
    void                WriteFontProperties( ScfPropertySet& rPropSet,
                            XclFontPropSetType eType, const Color* pFontColor = nullptr ) const;

private:
    /** Reads and sets height and flags. */
    void                ReadFontData2( XclImpStream& rStrm );
    /** Reads and sets height, flags, color, boldness, script, family and charset. */
    void                ReadFontData5( XclImpStream& rStrm );

    /** Reads and sets the font color. */
    void                ReadFontColor( XclImpStream& rStrm );

    /** Reads and sets a byte string as font name. */
    void                ReadFontName2( XclImpStream& rStrm );
    /** Reads and sets a Unicode string as font name. */
    void                ReadFontName8( XclImpStream& rStrm );

    /** Tests whether the font contains CJK or CTL characters.
        @descr  This is only a weak guess using preselected characters. */
    void                GuessScriptType();

private:
    XclFontData         maData;         /// All font attributes.
    bool                mbHasCharSet;   /// true = Font contains own character set info.
    bool                mbHasWstrn;     /// true = Font contains Western script characters.
    bool                mbHasAsian;     /// true = Font contains Asian script characters.
    bool                mbHasCmplx;     /// true = Font contains Complex script characters.
    bool                mbFontNameUsed; /// true = Font name, family, charset used.
    bool                mbHeightUsed;   /// true = Font height used.
    bool                mbColorUsed;    /// true = Color used.
    bool                mbWeightUsed;   /// true = Weight used.
    bool                mbEscapemUsed;  /// true = Escapement type used.
    bool                mbUnderlUsed;   /// true = Underline style used.
    bool                mbItalicUsed;   /// true = Italic used.
    bool                mbStrikeUsed;   /// true = Strikeout used.
    bool                mbOutlineUsed;  /// true = Outlined used.
    bool                mbShadowUsed;   /// true = Shadowed used.
};

/** Stores the data of all fonts occurred in an Excel file. */
class XclImpFontBuffer : protected XclImpRoot, private boost::noncopyable
{
public:
    explicit            XclImpFontBuffer( const XclImpRoot& rRoot );

    /** Clears all buffered data, used to set up for a new sheet. */
    void                Initialize();

    /** Returns the object that stores all contents of a FONT record. */
    const XclImpFont*   GetFont( sal_uInt16 nFontIndex ) const;
    /** Returns the application font data of this file, needed i.e. for column width. */
    inline const XclFontData& GetAppFontData() const { return maAppFont; }

    /** Reads a FONT record. */
    void                ReadFont( XclImpStream& rStrm );
    /** Reads an EFONT record (BIFF2 font color). */
    void                ReadEfont( XclImpStream& rStrm );

    /** Fills all font properties from a FONT record to the item set.
        @param rItemSet  The destination item set.
        @param eType  The type of Which-IDs.
        @param nFontIdx  The Excel index of the font.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                FillToItemSet( SfxItemSet& rItemSet, XclFontItemType eType,
                            sal_uInt16 nFontIdx, bool bSkipPoolDefs = false ) const;
    /** Writes all font properties to the passed property set.
        @param pFontColor  If set, overrides internal stored font color. */
    void                WriteFontProperties(
                            ScfPropertySet& rPropSet, XclFontPropSetType eType,
                            sal_uInt16 nFontIdx, const Color* pFontColor = nullptr ) const;
    /** Writes default font properties for form controls to the passed property set. */
    void                WriteDefaultCtrlFontProperties( ScfPropertySet& rPropSet ) const;

private:
    /** Updates the application default font. */
    void                UpdateAppFont( const XclFontData& rFontData, bool bHasCharSet );

private:
    std::vector< XclImpFont > maFontList; /// List of all FONT records in the Excel file.
    XclFontData         maAppFont;              /// Application font (for column width).
    XclImpFont          maFont4;                /// Built-in font with index 4.
    XclImpFont          maCtrlFont;             /// BIFF5 default form controls font (Helv,8pt,bold).
};

// FORMAT record - number formats =============================================

/** Stores all user defined number formats occurred in the file. */
class XclImpNumFmtBuffer : public XclNumFmtBuffer, protected XclImpRoot
{
public:
    explicit            XclImpNumFmtBuffer( const XclImpRoot& rRoot );

    /** Clears all buffered data, used to set up for a new sheet. */
    void                Initialize();

    /** Reads a FORMAT record. */
    void                ReadFormat( XclImpStream& rStrm );

    /** Read NumFmt from conditional format record */
    sal_uInt16          ReadCFFormat( XclImpStream& rStrm, bool bIFmt );

    /** Creates the number formats in the Calc document. */
    void                CreateScFormats();

    /** Returns the format key with the passed Excel index or NUMBERFORMAT_ENTRY_NOT_FOUND on error. */
    sal_uLong               GetScFormat( sal_uInt16 nXclNumFmt ) const;

    /** Fills an Excel number format to the passed item set.
        @param rItemSet  The destination item set.
        @param nXclNumFmt  The Excel number format index.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                FillToItemSet(
                            SfxItemSet& rItemSet, sal_uInt16 nXclNumFmt,
                            bool bSkipPoolDefs = false ) const;
    /** Fills a Calc number format to the passed item set.
        @param rItemSet  The destination item set.
        @param nScNumFmt  The Calc number formatter index of the format.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                FillScFmtToItemSet(
                            SfxItemSet& rItemSet, sal_uLong nScNumFmt,
                            bool bSkipPoolDefs = false ) const;

private:
    typedef ::std::map< sal_uInt16, sal_uLong > XclImpIndexMap;

    XclImpIndexMap      maIndexMap;     /// Maps Excel format indexes to Calc formats.
    sal_uInt16          mnNextXclIdx;   /// Index counter for BIFF2-BIFF4.
};

// XF, STYLE record - Cell formatting =========================================

/** Extends the XclCellProt struct for import.
    @descr  Provides functions to fill from Excel record data and to fill to item sets. */
struct XclImpCellProt : public XclCellProt
{
    /** Fills this struct with BIFF2 XF record data. */
    void                FillFromXF2( sal_uInt8 nNumFmt );
    /** Fills this struct with BIFF3-BIFF8 XF record data. */
    void                FillFromXF3( sal_uInt16 nProt );

    /** Inserts items representing this protection style into the item set.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                FillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs = false ) const;
};

/** Extends the XclCellAlign struct for import.
    @descr  Provides functions to fill from Excel record data and to fill to item sets. */
struct XclImpCellAlign : public XclCellAlign
{
    /** Fills this struct with BIFF2 XF record data. */
    void                FillFromXF2( sal_uInt8 nFlags );
    /** Fills this struct with BIFF3 XF record data. */
    void                FillFromXF3( sal_uInt16 nAlign );
    /** Fills this struct with BIFF4 XF record data. */
    void                FillFromXF4( sal_uInt16 nAlign );
    /** Fills this struct with BIFF5/BIFF7 XF record data. */
    void                FillFromXF5( sal_uInt16 nAlign );
    /** Fills this struct with BIFF8 XF record data. */
    void                FillFromXF8( sal_uInt16 nAlign, sal_uInt16 nMiscAttrib );
    /** Fills this struct with CF record data. */
    void                FillFromCF( sal_uInt16 nAlign, sal_uInt16 nMiscAttrib );

    /** Inserts items representing this alignment style into the item set.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                FillToItemSet( SfxItemSet& rItemSet, const XclImpFont* pFont, bool bSkipPoolDefs = false ) const;
};

/** Extends the XclCellBorder struct for import.
    @descr  Provides functions to fill from Excel record data and to fill to item sets. */
struct XclImpCellBorder : public XclCellBorder
{
    bool                mbLeftUsed;     /// true = Left line style used.
    bool                mbRightUsed;    /// true = Right line style used.
    bool                mbTopUsed;      /// true = Top line style used.
    bool                mbBottomUsed;   /// true = Bottom line style used.
    bool                mbDiagUsed;     /// true = Diagonal line style used.

    explicit            XclImpCellBorder();

    /** Sets outer line states and diagonal line states to used or unused. */
    void                SetUsedFlags( bool bOuterUsed, bool bDiagUsed );

    /** Fills this struct with BIFF2 XF record data. */
    void                FillFromXF2( sal_uInt8 nFlags );
    /** Fills this struct with BIFF3/BIFF4 XF record data. */
    void                FillFromXF3( sal_uInt32 nBorder );
    /** Fills this struct with BIFF5/BIFF7 XF record data. */
    void                FillFromXF5( sal_uInt32 nBorder, sal_uInt32 nArea );
    /** Fills this struct with BIFF8 XF record data. */
    void                FillFromXF8( sal_uInt32 nBorder1, sal_uInt32 nBorder2 );

    /** Fills this struct with BIFF8 CF (conditional format) record data. */
    void                FillFromCF8( sal_uInt16 nLineStyle, sal_uInt32 nLineColor, sal_uInt32 nFlags );

    /** Returns true, if any of the outer border lines is visible. */
    bool                HasAnyOuterBorder() const;

    /** Inserts a box item representing this border style into the item set.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                FillToItemSet(
                            SfxItemSet& rItemSet,
                            const XclImpPalette& rPalette,
                            bool bSkipPoolDefs = false ) const;
};

/** Extends the XclCellArea struct for import.
    @descr  Provides functions to fill from Excel record data and to fill to item sets. */
struct XclImpCellArea : public XclCellArea
{
    bool                mbForeUsed;     /// true = Foreground color used.
    bool                mbBackUsed;     /// true = Background color used.
    bool                mbPattUsed;     /// true = Pattern used.

    explicit            XclImpCellArea();

    /** Sets colors and pattern state to used or unused. */
    void                SetUsedFlags( bool bUsed );

    /** Fills this struct with BIFF2 XF record data. */
    void                FillFromXF2( sal_uInt8 nFlags );
    /** Fills this struct with BIFF3/BIFF4 XF record data. */
    void                FillFromXF3( sal_uInt16 nArea );
    /** Fills this struct with BIFF5/BIFF7 XF record data. */
    void                FillFromXF5( sal_uInt32 nArea );
    /** Fills this struct with BIFF8 XF record data. */
    void                FillFromXF8( sal_uInt32 nBorder2, sal_uInt16 nArea );

    /** Fills this struct with BIFF8 CF (conditional format) record data. */
    void                FillFromCF8( sal_uInt16 nPattern, sal_uInt16 nColor, sal_uInt32 nFlags );

    /** Inserts a brush item representing this area style into the item set.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                FillToItemSet(
                            SfxItemSet& rItemSet,
                            const XclImpPalette& rPalette,
                            bool bSkipPoolDefs = false ) const;
};

/** Represents an XF record index with additional information. */
class XclImpXFIndex
{
public:
    inline explicit     XclImpXFIndex( sal_uInt16 nXFIndex, bool bBoolCell = false ) :
                            mnXFIndex( nXFIndex ), mbBoolCell( bBoolCell ) {}

    inline sal_uInt16   GetXFIndex() const { return mnXFIndex; }
    inline bool         IsBoolCell() const { return mbBoolCell; }

private:
    sal_uInt16          mnXFIndex;      /// The XF record index.
    bool                mbBoolCell;     /// true = A Boolean value cell.
};

inline bool operator==( const XclImpXFIndex& rLeft, const XclImpXFIndex& rRight )
{ return (rLeft.GetXFIndex() == rRight.GetXFIndex()) && (rLeft.IsBoolCell() == rRight.IsBoolCell()); }

inline bool operator!=( const XclImpXFIndex& rLeft, const XclImpXFIndex& rRight )
{ return !(rLeft == rRight); }

/** Contains all data of a XF record and a Calc item set. */
class XclImpXF : public XclXFBase, protected XclImpRoot, private boost::noncopyable
{
public:
    explicit            XclImpXF( const XclImpRoot& rRoot );
    virtual             ~XclImpXF();

    /** Reads an XF record. */
    void                ReadXF( XclImpStream& rStrm );

    inline sal_uInt8    GetHorAlign() const { return maAlignment.mnHorAlign; }
    inline sal_uInt16   GetFontIndex() const { return mnXclFont; }

    /** Creates a Calc item set containing an item set with all cell properties.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items.
        @return  A read-only reference to the item set stored internally. */
    const ScPatternAttr& CreatePattern( bool bSkipPoolDefs = false );

    void                ApplyPatternToAttrList(
                            ::std::list<ScAttrEntry>& rAttrs, SCROW nRow1, SCROW nRow2,
                            sal_uInt32 nForceScNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND);

    /** Inserts all formatting attributes to the specified area in the Calc document.
        @param nForcedNumFmt  If not set to NUMBERFORMAT_ENTRY_NOT_FOUND, it will overwrite
        the number format of the XF. */
    void                ApplyPattern(
                            SCCOL nScCol1, SCROW nScRow1,
                            SCCOL nScCol2, SCROW nScRow2,
                            SCTAB nScTab,
                            sal_uLong nForceScNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND );

    /** Converts formatting information from BIFF2 cell record data directly. */
    static void         ApplyPatternForBiff2CellFormat(
                            const XclImpRoot& rRoot, const ScAddress& rScPos,
                            sal_uInt8 nFlags1, sal_uInt8 nFlags2, sal_uInt8 nFlags3 );

private:
    void                ReadXF2( XclImpStream& rStrm );
    void                ReadXF3( XclImpStream& rStrm );
    void                ReadXF4( XclImpStream& rStrm );
    void                ReadXF5( XclImpStream& rStrm );
    void                ReadXF8( XclImpStream& rStrm );

    /** Sets all "attribute used" flags according to the passed mask.
        @descr  In cell XFs, a set bit represents "used", in style XFs it is a cleared bit.
        Therefore mbCellXF must be set correctly before calling this method. */
    void                SetUsedFlags( sal_uInt8 nUsedFlags );

private:
    typedef ::std::unique_ptr< ScPatternAttr > ScPatternAttrPtr;

    ScPatternAttrPtr    mpPattern;          /// Calc item set.
    ScStyleSheet*       mpStyleSheet;       /// Calc cell style sheet.

    XclImpCellProt      maProtection;       /// Cell protection flags.
    XclImpCellAlign     maAlignment;        /// All alignment attributes.
    XclImpCellBorder    maBorder;           /// Border line style.
    XclImpCellArea      maArea;             /// Background area style.
    sal_uInt16          mnXclNumFmt;        /// Index to number format.
    sal_uInt16          mnXclFont;          /// Index to font record.
};

/** Contains all data of a cell style associated with an XF record. */
class XclImpStyle : protected XclImpRoot
{
public:
    explicit            XclImpStyle( const XclImpRoot& rRoot );

    /** Reads a STYLE record. */
    void                ReadStyle( XclImpStream& rStrm );

    inline const OUString& GetName() const { return maName; }
    inline sal_uInt16   GetXfId() const { return mnXfId; }
    inline bool         IsBuiltin() const { return mbBuiltin && (mnBuiltinId != EXC_STYLE_USERDEF); }
    inline sal_uInt8    GetBuiltinId() const { return mnBuiltinId; }
    inline sal_uInt8    GetLevel() const { return mnLevel; }

    /** Creates a cell style sheet and inserts it into the Calc document.
        @return  The pointer to the cell style sheet, or 0, if there is no style sheet. */
    ScStyleSheet*       CreateStyleSheet();
    /** Creates the Calc style sheet, if this is a user-defined style. */
    void                CreateUserStyle( const OUString& rFinalName );

private:
    OUString            maName;             /// Cell style name.
    sal_uInt16          mnXfId;             /// Formatting for this cell style.
    sal_uInt8           mnBuiltinId;        /// Identifier for builtin styles.
    sal_uInt8           mnLevel;            /// Level for builtin column/row styles.
    bool                mbBuiltin;          /// True = builtin style.
    bool                mbCustom;           /// True = customized builtin style.
    bool                mbHidden;           /// True = style not visible in GUI.

    OUString            maFinalName;        /// Final name used in the Calc document.
    ScStyleSheet*       mpStyleSheet;       /// Calc cell style sheet.
};

/** Contains all XF records occurred in the file.
    @descr  This class is able to read XF records (BIFF2 - BIFF8) and STYLE records (BIFF8). */
class XclImpXFBuffer : protected XclImpRoot, private boost::noncopyable
{
public:
    explicit            XclImpXFBuffer( const XclImpRoot& rRoot );

    /** Clears all buffered data, used to set up for a new sheet. */
    void                Initialize();

    /** Reads an XF record. */
    void                ReadXF( XclImpStream& rStrm );
    /** Reads a STYLE record. */
    void                ReadStyle( XclImpStream& rStrm );

    /** Returns the object that stores all contents of an XF record. */
    inline XclImpXF*    GetXF( sal_uInt16 nXFIndex )
                            { return (nXFIndex >= maXFList.size()) ? nullptr : maXFList.at(nXFIndex).get(); }

    inline const XclImpXF*    GetXF( sal_uInt16 nXFIndex ) const
                            { return (nXFIndex >= maXFList.size()) ? nullptr : maXFList.at(nXFIndex).get(); }

    /** Returns the index to the Excel font used in the specified XF record. */
    sal_uInt16          GetFontIndex( sal_uInt16 nXFIndex ) const;
    /** Returns the Excel font used in the specified XF record. */
    const XclImpFont*   GetFont( sal_uInt16 nXFIndex ) const;

    /** Creates all user defined style sheets. */
    void                CreateUserStyles();
    /** Creates a cell style sheet of the passed XF and inserts it into the Calc document.
        @return  The pointer to the cell style sheet, or 0, if there is no style sheet. */
    ScStyleSheet*       CreateStyleSheet( sal_uInt16 nXFIndex );

private:
    typedef std::vector< std::unique_ptr<XclImpStyle> >        XclImpStyleList;
    typedef ::std::map< sal_uInt16, XclImpStyle* >  XclImpStyleMap;

    std::vector< std::unique_ptr<XclImpXF> > maXFList; /// List of contents of all XF record.
    XclImpStyleList     maBuiltinStyles;    /// List of built-in cell styles.
    XclImpStyleList     maUserStyles;       /// List of user defined cell styles.
    XclImpStyleMap      maStylesByXf;       /// Maps XF records to cell styles.
};

// Buffer for XF indexes in cells =============================================

/** Contains an (encoded) XF index for a range of rows in a single column. */
class XclImpXFRange
{
    DECL_FIXEDMEMPOOL_NEWDEL( XclImpXFRange )

public:
    SCROW               mnScRow1;       /// The first row of an equal-formatted range.
    SCROW               mnScRow2;       /// The last row of an equal-formatted range.
    XclImpXFIndex       maXFIndex;      /// Extended XF index.

    inline explicit     XclImpXFRange( SCROW nScRow, const XclImpXFIndex& rXFIndex );
    inline explicit     XclImpXFRange( SCROW nFirstScRow, SCROW nLastScRow, const XclImpXFIndex& rXFIndex );

    /** Returns true, if nScRow is contained in own row range. */
    inline bool         Contains( SCROW nScRow ) const;

    /** Returns true, if the range has been expanded. */
    bool                Expand( SCROW nScRow, const XclImpXFIndex& rXFIndex );
    /** Returns true, if the range has been expanded. */
    bool                Expand( const XclImpXFRange& rNextRange );
};

inline XclImpXFRange::XclImpXFRange( SCROW nScRow, const XclImpXFIndex& rXFIndex ) :
    mnScRow1( nScRow ),
    mnScRow2( nScRow ),
    maXFIndex( rXFIndex )
{
}

inline XclImpXFRange::XclImpXFRange( SCROW nFirstScRow, SCROW nLastScRow, const XclImpXFIndex& rXFIndex ) :
    mnScRow1( nFirstScRow ),
    mnScRow2( nLastScRow ),
    maXFIndex( rXFIndex )
{
}

inline bool XclImpXFRange::Contains( SCROW nScRow ) const
{
    return (mnScRow1 <= nScRow) && (nScRow <= mnScRow2);
}

/** Contains the XF indexes for every used cell in a column. */
class XclImpXFRangeColumn : private boost::noncopyable
{
public:
    typedef std::vector< std::unique_ptr<XclImpXFRange> > IndexList;

    inline explicit     XclImpXFRangeColumn() {}

    IndexList::iterator begin() { return maIndexList.begin(); }
    IndexList::iterator end() { return maIndexList.end(); }

    /** Inserts a single row range into the list. */
    void                SetDefaultXF( const XclImpXFIndex& rXFIndex );

    /** Inserts a new (encoded) XF index (first try to expand the last range). */
    void                SetXF( SCROW nScRow, const XclImpXFIndex& rXFIndex );

private:
    /** Finds the previous and next row range from row position nScRow.
        @descr  If an XF still exists, it is contained in rpPrevRange. */
    void                Find(
                            XclImpXFRange*& rpPrevRange,
                            XclImpXFRange*& rpNextRange,
                            sal_uLong& rnNextIndex,
                            SCROW nScRow );

    /** Tries to concatenate a range with its predecessor.
        @descr  The ranges must have the same XF index and must not have a gap.
        The resulting range has the index nIndex-1. */
    void                TryConcatPrev( sal_uLong nIndex );

    /** Insert a range into the list at the specified index. */
    void                Insert(XclImpXFRange* pXFRange, sal_uLong nIndex);

private:
    IndexList maIndexList;    /// The list of XF index range.
};

/** Contains the XF indexes for every used cell in a single sheet. */
class XclImpXFRangeBuffer : protected XclImpRoot, private boost::noncopyable
{
public:
    explicit            XclImpXFRangeBuffer( const XclImpRoot& rRoot );
    virtual             ~XclImpXFRangeBuffer();

    /** Clears all buffered data, used to set up for a new sheet. */
    void                Initialize();

    /** Inserts a new XF index. */
    void                SetXF( const ScAddress& rScPos, sal_uInt16 nXFIndex );
    /** Inserts a new XF index for blank cells. */
    void                SetBlankXF( const ScAddress& rScPos, sal_uInt16 nXFIndex );
    /** Inserts a new XF index for boolean cells. */
    void                SetBoolXF( const ScAddress& rScPos, sal_uInt16 nXFIndex );
    /** Inserts a new XF index for all cells in a row. */
    void                SetRowDefXF( SCROW nScRow, sal_uInt16 nXFIndex );
    /** Inserts a new XF index for all cells in a column. */
    void                SetColumnDefXF( SCCOL nScCol, sal_uInt16 nXFIndex );

    /** Inserts a range of hyperlink cells. */
    void                SetHyperlink( const XclRange& rXclRange, const OUString& rUrl );

    /** Inserts the first cell of a merged cell range. */
    void                SetMerge( SCCOL nScCol, SCROW nScRow );
    /** Inserts a complete merged cell range. */
    void                SetMerge( SCCOL nScCol1, SCROW nScRow1, SCCOL nScCol2, SCROW nScRow2 );

    /** Applies styles and cell merging to the current sheet in the document. */
    void                Finalize();

private:
    /** Insertion mode of an XF index. */
    enum XclImpXFInsertMode
    {
        xlXFModeCell,               /// Filled cell.
        xlXFModeBoolCell,           /// Cell with a single Boolean value.
        xlXFModeBlank,              /// Blank cell.
        xlXFModeRow                 /// Row default XF.
    };

private:
    /** Inserts a new XF index for the specified cell type. */
    void                SetXF( const ScAddress& rScPos, sal_uInt16 nXFIndex, XclImpXFInsertMode eMode );

    /** Copies border of the last cell of the range to the first cell to keep it visible
        when the range is merged.
        @param nLine
        SvxBoxItemLine::RIGHT = copy most-right border of top row;
        SvxBoxItemLine::BOTTOM = copy most-bottom border of first column. */
    void                SetBorderLine( const ScRange& rRange, SCTAB nScTab, SvxBoxItemLine nLine );

private:
    typedef std::shared_ptr< XclImpXFRangeColumn > XclImpXFRangeColumnRef;
    typedef ::std::vector< XclImpXFRangeColumnRef >  XclImpXFRangeColumnVec;
    typedef ::std::pair< XclRange, OUString >        XclImpHyperlinkRange;
    typedef ::std::list< XclImpHyperlinkRange >      XclImpHyperlinkList;

    XclImpXFRangeColumnVec maColumns;       /// Array of column XF index buffers.
    XclImpHyperlinkList maHyperlinks;       /// Maps URLs to hyperlink cells.
    ScRangeList         maMergeList;        /// List of merged cell ranges.
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
