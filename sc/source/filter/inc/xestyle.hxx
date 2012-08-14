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

#ifndef SC_XESTYLE_HXX
#define SC_XESTYLE_HXX

#include <map>
#include <tools/mempool.hxx>
#include <tools/string.hxx>
#include <svl/zforlist.hxx>
#include <svl/nfkeytab.hxx>
#include <editeng/svxfont.hxx>
#include "xerecord.hxx"
#include "xlstyle.hxx"
#include "xeroot.hxx"
#include "conditio.hxx"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

/* ============================================================================
- Buffers for style records (PALETTE, FONT, FORMAT, XF, STYLE).
============================================================================ */

const sal_uInt16 EXC_ID_FONTLIST    = 0x8031;   /// For internal use only.
const sal_uInt16 EXC_ID_FORMATLIST  = 0x801E;   /// For internal use only.
const sal_uInt16 EXC_ID_XFLIST      = 0x8043;   /// For internal use only.
const sal_uInt16 EXC_ID_DXFS        = 0x9999;   /// For internal use only. TODO:moggi: find a better/correct value

// PALETTE record - color information =========================================

/** Different types of colors in a document. */
enum XclExpColorType
{
    EXC_COLOR_CELLTEXT,         /// Text in a cell.
    EXC_COLOR_CELLBORDER,       /// Border of a cell.
    EXC_COLOR_CELLAREA,         /// Background area of a cell.
    EXC_COLOR_CHARTTEXT,        /// Text color in a chart.
    EXC_COLOR_CHARTLINE,        /// Line in a chart.
    EXC_COLOR_CHARTAREA,        /// Area in a chart.
    EXC_COLOR_CTRLTEXT,         /// Text color in a form control.
    EXC_COLOR_GRID,              /// Spreadsheet grid color.
    EXC_COLOR_TABBG             /// Spreadsheet tab bg color.
};

// ----------------------------------------------------------------------------

class XclExpPaletteImpl;

/** Stores all used colors in the document.

    Supports color reduction to the maximum count of the current BIFF version.
    An instance of this class collects all colors in the conversion phase of
    the export, using the InsertColor() function. It returns a unique
    identidier for each passed color.

    After the entire document is converted, the Finalize() function will reduce
    the palette to the  number of colors supported by the current BIFF version.

    Then, in the streaming phase, the functions GetColorIndex() and
    GetMixedColors() return the real Excel palette index for all color
    identifiers.
 */
class XclExpPalette : public XclDefaultPalette, public XclExpRecord
{
public:
    explicit            XclExpPalette( const XclExpRoot& rRoot );
    virtual             ~XclExpPalette();

    /** Inserts the color into the list and updates weighting.
        @param nAutoDefault  The Excel palette index for automatic color.
        @return  A unique ID for this color. */
    sal_uInt32          InsertColor( const Color& rColor, XclExpColorType eType, sal_uInt16 nAutoDefault = 0 );
    /** Returns the color ID representing a fixed Excel palette index (i.e. for auto colors). */
    static sal_uInt32   GetColorIdFromIndex( sal_uInt16 nIndex );

    /** Reduces the color list to the maximum count of the current BIFF version. */
    void                Finalize();

    /** Returns the Excel palette index of the color with passed color ID. */
    sal_uInt16          GetColorIndex( sal_uInt32 nColorId ) const;

    /** Returns a foreground and background color for the two passed color IDs.
        @descr  If rnXclPattern contains a solid pattern, this function tries to find
        the two best fitting colors and a mix pattern (25%, 50% or 75%) for nForeColorId.
        This will result in a better approximation to the passed foreground color. */
    void                GetMixedColors(
                            sal_uInt16& rnXclForeIx, sal_uInt16& rnXclBackIx, sal_uInt8& rnXclPattern,
                            sal_uInt32 nForeColorId, sal_uInt32 nBackColorId ) const;

    /** Returns the RGB color data for a (non-zero-based) Excel palette entry.
        @return  The color from current or default palette or COL_AUTO, if nothing else found. */
    ColorData           GetColorData( sal_uInt16 nXclIndex ) const;
    /** Returns the color for a (non-zero-based) Excel palette entry.
        @return  The color from current or default palette or COL_AUTO, if nothing else found. */
    inline Color        GetColor( sal_uInt16 nXclIndex ) const
                            { return Color( GetColorData( nXclIndex ) ); }

    /** Saves the PALETTE record, if it differs from the default palette. */
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    /** Writes the contents of the PALETTE record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    typedef boost::shared_ptr< XclExpPaletteImpl > XclExpPaletteImplRef;
    XclExpPaletteImplRef mxImpl;
};

// FONT record - font information =============================================

class Font;
class SvxFont;

const size_t EXC_FONTLIST_NOTFOUND = static_cast< size_t >( -1 );

// ----------------------------------------------------------------------------

/** Static helper functions for font export. */
class XclExpFontHelper
{
public:
    /** Returns the script type of the first font item found in the item set and its parents. */
    static sal_Int16    GetFirstUsedScript(
                            const XclExpRoot& rRoot,
                            const SfxItemSet& rItemSet );

    /** Returns a VCL font object filled from the passed item set. */
    static Font         GetFontFromItemSet(
                            const XclExpRoot& rRoot,
                            const SfxItemSet& rItemSet,
                            sal_Int16 nScript );

    /** Returns true, if at least one font related item is set in the passed item set.
        @param bDeep  true = Searches in parent item sets too. */
    static bool         CheckItems(
                            const XclExpRoot& rRoot,
                            const SfxItemSet& rItemSet,
                            sal_Int16 nScript,
                            bool bDeep );

private:
                            XclExpFontHelper();
                            ~XclExpFontHelper();
};

// ----------------------------------------------------------------------------

/** Stores all data of an Excel font and provides export of FONT records. */
class XclExpFont : public XclExpRecord, protected XclExpRoot
{
public:
    explicit            XclExpFont( const XclExpRoot& rRoot,
                            const XclFontData& rFontData, XclExpColorType eColorType );

    /** Returns read-only access to font data. */
    inline const XclFontData& GetFontData() const { return maData; }
    /** Returns the font color identifier. */
    inline sal_uInt32   GetFontColorId() const { return mnColorId; }
    /** Compares this font with the passed font data.
        @param nHash  The hash value calculated from the font data. */
    virtual bool        Equals( const XclFontData& rFontData, sal_uInt32 nHash ) const;

    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    /** Writes the contents of the FONT record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    XclFontData         maData;         /// All font attributes.
    sal_uInt32          mnColorId;      /// Unique color ID for text color.
    sal_uInt32          mnHash;         /// Hash value for fast comparison.
};

// ----------------------------------------------------------------------------

/** Used as placeholder for font index 4, which is not used in Excel. */
class XclExpBlindFont : public XclExpFont
{
public:
    explicit            XclExpBlindFont( const XclExpRoot& rRoot );

    /** Returns always false to never find this font while searching the font list. */
    virtual bool        Equals( const XclFontData& rFontData, sal_uInt32 nHash ) const;

    /** Skips writing this record. */
    virtual void        Save( XclExpStream& rStrm );
};

// ----------------------------------------------------------------------------

class ScPatternAttr;

/** Stores the data of all fonts used in the document. */
class XclExpFontBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpFontBuffer( const XclExpRoot& rRoot );

    /** Returns the specified font from font list. */
    const XclExpFont*   GetFont( sal_uInt16 nXclFont ) const;
    /** Returns the application font data of this file, needed e.g. for column width. */
    const XclFontData&  GetAppFontData() const;

    /** Inserts a new font with the passed font data into the buffer if not present.
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16          Insert( const XclFontData& rFontData,
                            XclExpColorType eColorType, bool bAppFont = false );
    /** Inserts the font into the buffer if not present.
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16          Insert( const Font& rFont,
                            XclExpColorType eColorType, bool bAppFont = false );
    /** Inserts the SvxFont into the buffer if not present, e.g. where escapements are used.
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16          Insert( const SvxFont& rFont,
                            XclExpColorType eColorType, bool bAppFont = false );
    /** Inserts the font contained in the passed item set into the buffer, if not present.
        @param nScript  The script type of the font properties to be used.
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16          Insert( const SfxItemSet& rItemSet, sal_Int16 nScript,
                            XclExpColorType eColorType, bool bAppFont = false );

    /** Writes all FONT records contained in this buffer. */
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    /** Initializes the default fonts for the current BIFF version. */
    void                InitDefaultFonts();
    /** Tries to find the passed font and returns the current list index. */
    size_t              Find( const XclFontData& rFontData );

private:
    typedef XclExpRecordList< XclExpFont >  XclExpFontList;
    typedef XclExpFontList::RecordRefType   XclExpFontRef;

    XclExpFontList      maFontList;     /// List of all FONT records.
    size_t              mnXclMaxSize;   /// Maximum number of fonts.
};

// FORMAT record - number formats =============================================

/** Stores a core number format index with corresponding Excel format index. */
struct XclExpNumFmt
{
    sal_uLong           mnScNumFmt;     /// Core index of the number format.
    sal_uInt16          mnXclNumFmt;    /// Resulting Excel format index.
    rtl::OUString       maNumFmtString; /// format string

    inline explicit     XclExpNumFmt( sal_uLong nScNumFmt, sal_uInt16 nXclNumFmt, const rtl::OUString& rFrmt ) :
                            mnScNumFmt( nScNumFmt ), mnXclNumFmt( nXclNumFmt ), maNumFmtString( rFrmt ) {}

    void SaveXml( XclExpXmlStream& rStrm );
};

// ----------------------------------------------------------------------------

class SvNumberFormatter;
typedef ::std::auto_ptr< SvNumberFormatter >    SvNumberFormatterPtr;

/** Stores all number formats used in the document. */
class XclExpNumFmtBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpNumFmtBuffer( const XclExpRoot& rRoot );
    virtual             ~XclExpNumFmtBuffer();

    /** Returns the core index of the current standard number format. */
    inline sal_uLong    GetStandardFormat() const { return mnStdFmt; }

    /** Inserts a number format into the format buffer.
        @param nScNumFmt  The core index of the number format.
        @return  The resulting Excel format index. */
    sal_uInt16          Insert( sal_uLong nScNumFmt );

    /** Writes all FORMAT records contained in this buffer. */
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    /** Writes the FORMAT record with index nXclIx and format string rFormatStr. */
    void                WriteFormatRecord( XclExpStream& rStrm, sal_uInt16 nXclNumFmt, const String& rFormatStr );
    /** Writes the FORMAT record represented by rFormat. */
    void                WriteFormatRecord( XclExpStream& rStrm, const XclExpNumFmt& rFormat );

    String              GetFormatCode ( sal_uInt16 nScNumFmt );

private:
    typedef ::std::vector< XclExpNumFmt >           XclExpNumFmtVec;

    SvNumberFormatterPtr mxFormatter;   /// Special number formatter for conversion.
    XclExpNumFmtVec     maFormatMap;    /// Maps core formats to Excel indexes.
    boost::scoped_ptr<NfKeywordTable>   mpKeywordTable; /// Replacement table.
    sal_uLong           mnStdFmt;       /// Key for standard number format.
    sal_uInt16          mnXclOffset;    /// Offset to first user defined format.
};

// XF, STYLE record - Cell formatting =========================================

/** Extends the XclCellProt struct for export.
    @descr  Provides functions to fill from item sets and to fill to Excel record data. */
struct XclExpCellProt : public XclCellProt
{
    /** Fills the protection attributes from the passed item set.
        @return  true = At least one protection item is set. */
    bool                FillFromItemSet( const SfxItemSet& rItemSet, bool bStyle = false );

    /** Fills the data to the passed fields of a BIFF3-BIFF8 XF record. */
    void                FillToXF3( sal_uInt16& rnProt ) const;

    void                SaveXml( XclExpXmlStream& rStrm ) const;
};

// ----------------------------------------------------------------------------

/** Extends the XclCellAlign struct for export.
    @descr  Provides functions to fill from item sets and to fill to Excel record data. */
struct XclExpCellAlign : public XclCellAlign
{
    /** Fills the alignment attributes from the passed item set.
        @descr  Fills only the attributes exported in the passed BIFF version.
        @param bForceLineBreak  true = Set line break flag unconditionally.
        @return  true = At least one alignment item is set. */
    bool                FillFromItemSet( const SfxItemSet& rItemSet,
                            bool bForceLineBreak, XclBiff eBiff, bool bStyle = false );

    /** Fills the data to the passed fields of a BIFF5/BIFF7 XF record. */
    void                FillToXF5( sal_uInt16& rnAlign ) const;
    /** Fills the data to the passed fields of a BIFF8 XF record. */
    void                FillToXF8( sal_uInt16& rnAlign, sal_uInt16& rnMiscAttrib ) const;

    void                SaveXml( XclExpXmlStream& rStrm ) const;
};

// ----------------------------------------------------------------------------

/** Extends the XclCellBorder struct for export.
    @descr  Provides functions to fill from item sets and to fill to Excel record data. */
struct XclExpCellBorder : public XclCellBorder
{
    sal_uInt32          mnLeftColorId;      /// Color ID for left line.
    sal_uInt32          mnRightColorId;     /// Color ID for right line.
    sal_uInt32          mnTopColorId;       /// Color ID for top line.
    sal_uInt32          mnBottomColorId;    /// Color ID for bottom line.
    sal_uInt32          mnDiagColorId;      /// Color ID for diagonal line(s).

    explicit            XclExpCellBorder();

    /** Fills the border attributes from the passed item set.
        @descr  Fills only the attributes exported in the passed BIFF version.
        @return  true = At least one border item is set. */
    bool                FillFromItemSet( const SfxItemSet& rItemSet,
                            XclExpPalette& rPalette, XclBiff eBiff, bool bStyle = false );
    /** Fills the mn***Color base members from the mn***ColorId members. */
    void                SetFinalColors( const XclExpPalette& rPalette );

    /** Fills the data to the passed fields of a BIFF5/BIFF7 XF record. */
    void                FillToXF5( sal_uInt32& rnBorder, sal_uInt32& rnArea ) const;
    /** Fills the data to the passed fields of a BIFF8 XF record. */
    void                FillToXF8( sal_uInt32& rnBorder1, sal_uInt32& rnBorder2 ) const;

    /** Fills the data to the passed fields of a BIFF8 CF (conditional format) record. */
    void                FillToCF8( sal_uInt16& rnLine, sal_uInt32& rnColor ) const;

    void                SaveXml( XclExpXmlStream& rStrm ) const;
};

// ----------------------------------------------------------------------------

/** Extends the XclCellArea struct for export.
    @descr  Provides functions to fill from item sets and to fill to Excel record data. */
struct XclExpCellArea : public XclCellArea
{
    sal_uInt32          mnForeColorId;  /// Foreground color ID.
    sal_uInt32          mnBackColorId;  /// Background color ID.

    explicit            XclExpCellArea();

    /** Fills the area attributes from the passed item set.
        @return  true = At least one area item is set. */
    bool                FillFromItemSet(
                            const SfxItemSet& rItemSet, XclExpPalette& rPalette,
                            bool bStyle = false );
    /** Fills the mn***Color base members from the mn***ColorId members. */
    void                SetFinalColors( const XclExpPalette& rPalette );

    /** Fills the data to the passed fields of a BIFF5/BIFF7 XF record. */
    void                FillToXF5( sal_uInt32& rnArea ) const;
    /** Fills the data to the passed fields of a BIFF8 XF record. */
    void                FillToXF8( sal_uInt32& rnBorder2, sal_uInt16& rnArea ) const;

    /** Fills the data to the passed fields of a BIFF8 CF (conditional format) record. */
    void                FillToCF8( sal_uInt16& rnPattern, sal_uInt16& rnColor ) const;

    void                SaveXml( XclExpXmlStream& rStrm ) const;
};

struct XclExpColor
{
    Color maColor;

    bool FillFromItemSet( const SfxItemSet& rItemSet );

    void SaveXml( XclExpXmlStream& rStrm ) const;
};

// ----------------------------------------------------------------------------

/** A combination of unique XF identifier with real Excel XF index. */
struct XclExpXFId
{
    sal_uInt32          mnXFId;         /// Temporary XF identifier.
    sal_uInt16          mnXFIndex;      /// Real Excel XF index.

    explicit            XclExpXFId();
    explicit            XclExpXFId( sal_uInt32 nXFId );

    /** Converts the XF identifier in mnXFId to an Excel XF index and stores it in mnXFIndex. */
    void                ConvertXFIndex( const XclExpRoot& rRoot );
};

// ----------------------------------------------------------------------------

class SfxStyleSheetBase;

/** Represents an XF record which contains all formatting data of a cell or cell style. */
class XclExpXF : public XclXFBase, public XclExpRecord, protected XclExpRoot
{
public:
    /** Constructs a cell XF record from the passed Calc cell formatting. */
    explicit            XclExpXF(
                            const XclExpRoot& rRoot,
                            const ScPatternAttr& rPattern,
                            sal_Int16 nScript,
                            sal_uLong nScForceNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND,
                            sal_uInt16 nForceXclFont = EXC_FONT_NOTFOUND,
                            bool bForceLineBreak = false );
    /** Constructs a style XF record from the passed cell style sheet. */
    explicit            XclExpXF(
                            const XclExpRoot& rRoot,
                            const SfxStyleSheetBase& rStyleSheet );

    /** Returns the cell protection settings of this XF. */
    const XclExpCellProt& GetProtectionData() const { return maProtection; }
    /** Returns the alignment settings of this XF. */
    const XclExpCellAlign& GetAlignmentData() const { return maAlignment; }
    /** Returns the cell border settings of this XF. */
    const XclExpCellBorder& GetBorderData() const { return maBorder; }
    /** Returns the cell fill settings of this XF. */
    const XclExpCellArea& GetAreaData() const { return maArea; }

    /** Returns true, if this XF record represents the passed cell formatting.
        @descr  Searches for cell XFs only. */
    bool                Equals(
                            const ScPatternAttr& rPattern,
                            sal_uLong nScForceNumFmt,
                            sal_uInt16 nForceXclFont,
                            bool bForceLineBreak ) const;

    /** Returns true, if this XF record represents the passed style.
        @descr  Searches for style XFs only. */
    bool                Equals( const SfxStyleSheetBase& rStyleSheet ) const;

    /** Sets the resulting Excel palette index from all used color IDs (border and area). */
    void                SetFinalColors();

    /** Returns true, if this XF record is completely equal to the passed. */
    bool                Equals( const XclExpXF& rCmpXF ) const;

    void                SetXmlIds( sal_uInt32 nBorderId, sal_uInt32 nFillId );

    virtual void        SaveXml( XclExpXmlStream& rStrm );

protected:
    explicit            XclExpXF( const XclExpRoot& rRoot, bool bCellXF );

protected:  // access for XclExpDefaultXF
    const SfxItemSet*   mpItemSet;          /// Pointer to the item set (we do not own it).

    XclExpCellProt      maProtection;       /// Cell protection flags.
    XclExpCellAlign     maAlignment;        /// All alignment attributes.
    XclExpCellBorder    maBorder;           /// Border line style.
    XclExpCellArea      maArea;             /// Background area style.
    sal_uInt32          mnParentXFId;       /// XF ID of parent XF record.
    sal_uLong               mnScNumFmt;         /// Calc number format index.
    sal_uInt16          mnXclFont;          /// Excel font index.
    sal_uInt16          mnXclNumFmt;        /// Excel number format index.
    sal_Int32           mnBorderId;         /// OOXML Border Index
    sal_Int32           mnFillId;           /// OOXML Fill Index

private:
    using               XclXFBase::Equals;

    /** Initializes with default values. */
    void                InitDefault();
    /** Fills all members from the passed item set.
        @param bDefStyle  true = This is the "Default"/"Normal" style - needs special handling. */
    void                Init(
                            const SfxItemSet& rItemSet,
                            sal_Int16 nScript,
                            sal_uLong nForceScNumFmt,
                            sal_uInt16 nForceXclFont,
                            bool bForceLineBreak,
                            bool bDefStyle );

    /** Returns the bits specifying the used attributes.
        @descr  In cell XFs a set bit means a used attribute, in style XF a cleared
        bit means a used attribute. This method regards the cell/style state.
        @return  The mask based on bit 0 (not yet bit-shifted as needed for export). */
    sal_uInt8           GetUsedFlags() const;

    void                WriteBody5( XclExpStream& rStrm );
    void                WriteBody8( XclExpStream& rStrm );

    /** Writes the contents of the XF record. */
    virtual void        WriteBody( XclExpStream& rStrm );
};

// ----------------------------------------------------------------------------

/** Represents a default XF record. Supports methods to set attributes directly. */
class XclExpDefaultXF : public XclExpXF
{
public:
    explicit            XclExpDefaultXF( const XclExpRoot& rRoot, bool bCellXF );

    /** Sets the Excel font index. */
    void                SetFont( sal_uInt16 nXclFont );
    /** Sets the Excel number format index. */
    void                SetNumFmt( sal_uInt16 nXclNumFmt );
};

// ----------------------------------------------------------------------------

/** Represents a STYLE record containing the data of a cell style.
    @descr  The calss is able to store built-in and user-defined styles. */
class XclExpStyle : public XclExpRecord
{
public:
    explicit            XclExpStyle( sal_uInt32 nXFId, const String& rStyleName );
    explicit            XclExpStyle( sal_uInt32 nXFId, sal_uInt8 nStyleId, sal_uInt8 nLevel = EXC_STYLE_NOLEVEL );

    /** Returns true, if this record represents an Excel built-in style. */
    inline bool         IsBuiltIn() const { return mnStyleId != EXC_STYLE_USERDEF; }

    inline const String&    GetName() const { return maName; }

    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    /** Writes the contents of the STYLE record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    String              maName;         /// Name of the cell style.
    XclExpXFId          maXFId;         /// XF identifier for style formatting.
    sal_uInt8           mnStyleId;      /// Built-in style identifier.
    sal_uInt8           mnLevel;        /// Outline level for RowLevel and ColLevel styles.
};

// ----------------------------------------------------------------------------

/** Stores all XF records (cell formats and cell styles) in the document.

    Stores also the names of user defined cell styles (STYLE records). Supports
    reduction to the maximum count of XF records of the current BIFF version.

    An instance of this class collects all XF records in the conversion phase
    of the export, using the Insert() and InsertStyle() functions. It returns a
    unique identidier for each XF record.

    After the entire document is converted, the Finalize() function will reduce
    the list to the number of XF records supported by the current BIFF version.

    Then, in the streaming phase, the function GetXFIndex() returns the real
    Excel XF index for all XF identifiers.
 */
class XclExpXFBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpXFBuffer( const XclExpRoot& rRoot );

    /** Inserts predefined built-in styles and user-defined styles. */
    void                Initialize();

    /** Finds or creates a cell XF record for the passed item set.
        @return  A unique XF record ID. */
    sal_uInt32          Insert( const ScPatternAttr* pPattern, sal_Int16 nScript );
    /** Finds or creates a cell XF record for the passed item set.
        @param nForceXclFont  The font to be exported. If not equal to EXC_FONT_NOTFOUND,
            this font index will be used unconditionally and the cell font will be ignored.
        @param bForceLineBreak  true = Set line break flag unconditionally.
            This is required for cells that contain multi-line text.
        @return  A unique XF record ID. */
    sal_uInt32          InsertWithFont(
                            const ScPatternAttr* pPattern, sal_Int16 nScript,
                            sal_uInt16 nForceXclFont,
                            bool bForceLineBreak );
    /** Finds or creates a cell XF record for the passed item set, with custom number format.
        @param nXFFlags  Additional flags allowing to control the creation of an XF.
        @param nForceScNumFmt  The number format to be exported, e.g. formula
            result type. This format will always overwrite the cell's number format.
        @param bForceLineBreak  true = Set line break flag unconditionally.
            This is required for cells that contain multi-line text.
        @return  A unique XF record ID. */
    sal_uInt32          InsertWithNumFmt(
                            const ScPatternAttr* pPattern, sal_Int16 nScript,
                            sal_uLong nForceScNumFmt,
                            bool bForceLineBreak );
    /** Inserts the passed cell style. Creates a style XF record and a STYLE record.
        @return  A unique XF record ID. */
    sal_uInt32          InsertStyle( const SfxStyleSheetBase* pStyleSheet );
    /** Returns the XF identifier representing a fixed Excel XF index (e.g. for built-in XFs). */
    static sal_uInt32   GetXFIdFromIndex( sal_uInt16 nXFIndex );
    /** Returns the XF identifier representing the default cell XF. */
    static sal_uInt32   GetDefCellXFId();

    /** Returns an XF record by its unique identifier. */
    const XclExpXF*     GetXFById( sal_uInt32 nXFId ) const;

    /** Reduces the XF record list to the maximum allowed number of records. */
    void                Finalize();

    /** Returns the Excel XF index of the XF record with passed XF ID. */
    sal_uInt16          GetXFIndex( sal_uInt32 nXFId ) const;

    sal_Int32           GetXmlStyleIndex( sal_uInt32 nXFId ) const;
    sal_Int32           GetXmlCellIndex( sal_uInt32 nXFId ) const;

    /** Writes all XF records contained in this buffer. */
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    typedef XclExpRecordList< XclExpXF >    XclExpXFList;
    typedef XclExpXFList::RecordRefType     XclExpXFRef;
    typedef XclExpRecordList< XclExpStyle > XclExpStyleList;

private:
    /** Returns the XF ID of the cell XF containing the passed format. */
    sal_uInt32          FindXF( const ScPatternAttr& rPattern, sal_uLong nForceScNumFmt,
                            sal_uInt16 nForceXclFont, bool bForceLineBreak ) const;
    /** Returns the XF ID of the style XF containing the passed style. */
    sal_uInt32          FindXF( const SfxStyleSheetBase& rStyleSheet ) const;

    /** Returns the XF ID of a built-in style XF, searches by style identifier. */
    sal_uInt32          FindBuiltInXF( sal_uInt8 nStyleId, sal_uInt8 nLevel = EXC_STYLE_NOLEVEL ) const;

    /** Tries to find the XF record containing the passed format or inserts a new record.
        @return  The XF record ID. */
    sal_uInt32          InsertCellXF( const ScPatternAttr* pPattern, sal_Int16 nScript,
                            sal_uLong nForceScNumFmt,
                            sal_uInt16 nForceXclFont, bool bForceLineBreak );
    /** Inserts the passed cell style. Creates a style XF record and a STYLE record.
        @return  The XF record ID. */
    sal_uInt32          InsertStyleXF( const SfxStyleSheetBase& rStyleSheet );

    /** Inserts an XF and a STYLE record for all user defined style sheets. */
    void                InsertUserStyles();

    /** Inserts a built-in XF record without a STYLE record and returns the XF ID.
        @param bCreateStyleRec  true = Creates the related STYLE record. */
    sal_uInt32          AppendBuiltInXF( XclExpXFRef xXF,
                            sal_uInt8 nStyleId, sal_uInt8 nLevel = EXC_STYLE_NOLEVEL );
    /** Inserts a built-in XF and STYLE record and returns the XF ID.
        @param bCreateStyleRec  true = Creates the related STYLE record. */
    sal_uInt32          AppendBuiltInXFWithStyle( XclExpXFRef xXF,
                            sal_uInt8 nStyleId, sal_uInt8 nLevel = EXC_STYLE_NOLEVEL );

    /** Inserts all default XF and STYLE records. */
    void                InsertDefaultRecords();

    /** Appends a XF index to the internal ID<->index maps. */
    void                AppendXFIndex( sal_uInt32 nXFId );

    void                AddBorderAndFill( const XclExpXF& rXF );
    void                SaveXFXml( XclExpXmlStream& rStrm, XclExpXF& rXF );

private:
    /** Extended info about a built-in XF. */
    struct XclExpBuiltInInfo
    {
        sal_uInt8           mnStyleId;          /// Built-in style identifier.
        sal_uInt8           mnLevel;            /// Level for RowLevel/ColLevel styles.
        bool                mbPredefined;       /// true = XF still predefined.
        bool                mbHasStyleRec;      /// true = STYLE record created.
        explicit            XclExpBuiltInInfo();
    };
    typedef ::std::map< sal_uInt32, XclExpBuiltInInfo > XclExpBuiltInMap;
    typedef ::std::vector< XclExpCellBorder >           XclExpBorderList;
    typedef ::std::vector< XclExpCellArea >             XclExpFillList;

    XclExpXFList        maXFList;           /// List of all XF records.
    XclExpStyleList     maStyleList;        /// List of all STYLE records.
    XclExpBuiltInMap    maBuiltInMap;       /// Contained elements describe built-in XFs.
    ScfUInt16Vec        maXFIndexVec;       /// Maps XF IDs to XF indexes.
    ScfUInt16Vec        maStyleIndexes;     /// Maps XF IDs to OOXML Style indexes
    ScfUInt16Vec        maCellIndexes;      /// Maps XF IDs to OOXML Cell indexes
    XclExpXFList        maSortedXFList;     /// List of XF records in XF index order.
    XclExpBorderList    maBorders;          /// List of borders used by XF records
    XclExpFillList      maFills;            /// List of fills used by XF records

};

struct XclDxfStyle
{
};

class XclExpDxf : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpDxf( const XclExpRoot& rRoot, XclExpCellAlign* pAlign, XclExpCellBorder* pBorder,
            XclExpFont* pFont, XclExpNumFmt* pNumberFmt, XclExpCellProt* pProt, XclExpColor* pColor);
    virtual ~XclExpDxf();

    virtual void SaveXml( XclExpXmlStream& rStrm );

private:
    XclExpCellAlign* mpAlign;
    XclExpCellBorder* mpBorder;
    XclExpFont* mpFont;
    XclExpNumFmt* mpNumberFmt;
    XclExpCellProt* mpProt;
    XclExpColor* mpColor;
};

class XclExpDxfs : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpDxfs( const XclExpRoot& rRoot );
    virtual ~XclExpDxfs() {}

    sal_Int32 GetDxfId(const rtl::OUString& rName);

    virtual void SaveXml( XclExpXmlStream& rStrm);
private:
    typedef boost::ptr_vector<XclExpDxf> DxfContainer;
    std::map<rtl::OUString, sal_Int32> maStyleNameToDxfId;
    DxfContainer maDxf;
    SvNumberFormatterPtr mxFormatter;   /// Special number formatter for conversion.
    boost::scoped_ptr<NfKeywordTable>   mpKeywordTable; /// Replacement table.
};

// ============================================================================

class XclExpXmlStyleSheet : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpXmlStyleSheet( const XclExpRoot& rRoot );

    virtual void        SaveXml( XclExpXmlStream& rStrm );
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
