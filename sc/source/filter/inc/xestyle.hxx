/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xestyle.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:30:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XESTYLE_HXX
#define SC_XESTYLE_HXX

#include <map>

#ifndef _SVMEMPOOL_HXX
#include <tools/mempool.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_NFKEYTAB_HXX
#include <svtools/nfkeytab.hxx>
#endif
#ifndef _SVX_SVXFONT_HXX
#include <svx/svxfont.hxx>
#endif

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif
#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif
#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

/* ============================================================================
- Buffers for style records (PALETTE, FONT, FORMAT, XF, STYLE).
============================================================================ */

const sal_uInt16 EXC_ID_FONTLIST    = 0x8031;   /// For internal use only.
const sal_uInt16 EXC_ID_FORMATLIST  = 0x801E;   /// For internal use only.
const sal_uInt16 EXC_ID_XFLIST      = 0x8043;   /// For internal use only.

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
    EXC_COLOR_GRID              /// Spreadsheet grid color.
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

private:
    /** Writes the contents of the PALETTE record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    typedef ScfRef< XclExpPaletteImpl > XclExpPaletteImplRef;
    XclExpPaletteImplRef mxImpl;
};

// FONT record - font information =============================================

class Font;
class SvxFont;

const size_t EXC_FONTLIST_NOTFOUND = static_cast< size_t >( -1 );

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
    /** Inserts the font contained in rPattern into the buffer if not present.
        @param nScript  The script type of the font properties to be used.
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16          Insert( const ScPatternAttr& rPattern, sal_Int16 nScript,
                            XclExpColorType eColorType, bool bAppFont = false );

    /** Writes all FONT records contained in this buffer. */
    virtual void        Save( XclExpStream& rStrm );

    /** Returns the script type of the first font item found in the item set and its parents. */
    static sal_Int16    GetFirstUsedScript( const SfxItemSet& rItemSet );

    /** Returns a VCL font object filled from the passed item set. */
    static Font         GetFontFromItemSet( const SfxItemSet& rItemSet, sal_Int16 nScript );
    /** Returns true, if at least one font related item is set in the passed item set.
        @param bDeep  true = Searches in parent item sets too. */
    static bool         CheckItems( const SfxItemSet& rItemSet, sal_Int16 nScript, bool bDeep );

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
    ULONG               mnScNumFmt;     /// Core index of the number format.
    sal_uInt16          mnXclNumFmt;    /// Resulting Excel format index.

    inline explicit     XclExpNumFmt( ULONG nScNumFmt, sal_uInt16 nXclNumFmt ) :
                            mnScNumFmt( nScNumFmt ), mnXclNumFmt( nXclNumFmt ) {}
};

// ----------------------------------------------------------------------------

class SvNumberFormatter;

/** Stores all number formats used in the document. */
class XclExpNumFmtBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpNumFmtBuffer( const XclExpRoot& rRoot );
    virtual             ~XclExpNumFmtBuffer();

    /** Returns the core index of the current standard number format. */
    inline ULONG        GetStandardFormat() const { return mnStdFmt; }

    /** Inserts a number format into the format buffer.
        @param nScNumFmt  The core index of the number format.
        @return  The resulting Excel format index. */
    sal_uInt16          Insert( ULONG nScNumFmt );

    /** Writes all FORMAT records contained in this buffer. */
    virtual void        Save( XclExpStream& rStrm );

private:
    /** Writes the FORMAT record with index nXclIx and format string rFormatStr. */
    void                WriteFormatRecord( XclExpStream& rStrm, sal_uInt16 nXclNumFmt, const String& rFormatStr );
    /** Writes the FORMAT record represented by rFormat. */
    void                WriteFormatRecord( XclExpStream& rStrm, const XclExpNumFmt& rFormat );

private:
    typedef ::std::auto_ptr< SvNumberFormatter >    SvNumberFormatterPtr;
    typedef ::std::vector< XclExpNumFmt >           XclExpNumFmtVec;
    typedef NfKeywordTable*                         NfKeywordTablePtr;

    SvNumberFormatterPtr mxFormatter;   /// Special number formatter for conversion.
    XclExpNumFmtVec     maFormatMap;    /// Maps core formats to Excel indexes.
    NfKeywordTablePtr   mpKeywordTable; /// Replacement table.
    ULONG               mnStdFmt;       /// Key for standard number format.
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

#if 0
    /** Fills the data to the passed fields of a BIFF2 XF record. */
    void                FillToXF2( sal_uInt8& rnNumFmt ) const;
#endif
    /** Fills the data to the passed fields of a BIFF3-BIFF8 XF record. */
    void                FillToXF3( sal_uInt16& rnProt ) const;
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

#if 0
    /** Fills the data to the passed fields of a BIFF2 XF record. */
    void                FillToXF2( sal_uInt8& rnFlags ) const;
    /** Fills the data to the passed fields of a BIFF3 XF record. */
    void                FillToXF3( sal_uInt16& rnAlign ) const;
    /** Fills the data to the passed fields of a BIFF4 XF record. */
    void                FillToXF4( sal_uInt16& rnAlign ) const;
#endif
    /** Fills the data to the passed fields of a BIFF5/BIFF7 XF record. */
    void                FillToXF5( sal_uInt16& rnAlign ) const;
    /** Fills the data to the passed fields of a BIFF8 XF record. */
    void                FillToXF8( sal_uInt16& rnAlign, sal_uInt16& rnMiscAttrib ) const;
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

#if 0
    /** Fills the data to the passed fields of a BIFF2 XF record. */
    void                FillToXF2( sal_uInt8& rnFlags ) const;
    /** Fills the data to the passed fields of a BIFF3/BIFF4 XF record. */
    void                FillToXF3( sal_uInt32& rnBorder ) const;
#endif
    /** Fills the data to the passed fields of a BIFF5/BIFF7 XF record. */
    void                FillToXF5( sal_uInt32& rnBorder, sal_uInt32& rnArea ) const;
    /** Fills the data to the passed fields of a BIFF8 XF record. */
    void                FillToXF8( sal_uInt32& rnBorder1, sal_uInt32& rnBorder2 ) const;

    /** Fills the data to the passed fields of a BIFF8 CF (conditional format) record. */
    void                FillToCF8( sal_uInt16& rnLine, sal_uInt32& rnColor ) const;
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

#if 0
    /** Fills the data to the passed fields of a BIFF2 XF record. */
    void                FillToXF2( sal_uInt8& rnFlags ) const;
    /** Fills the data to the passed fields of a BIFF3/BIFF4 XF record. */
    void                FillToXF3( sal_uInt16& rnArea ) const;
#endif
    /** Fills the data to the passed fields of a BIFF5/BIFF7 XF record. */
    void                FillToXF5( sal_uInt32& rnArea ) const;
    /** Fills the data to the passed fields of a BIFF8 XF record. */
    void                FillToXF8( sal_uInt32& rnBorder2, sal_uInt16& rnArea ) const;

    /** Fills the data to the passed fields of a BIFF8 CF (conditional format) record. */
    void                FillToCF8( sal_uInt16& rnPattern, sal_uInt16& rnColor ) const;
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
                            ULONG nScForceNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND,
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
                            ULONG nScForceNumFmt,
                            sal_uInt16 nForceXclFont,
                            bool bForceLineBreak ) const;

    /** Returns true, if this XF record represents the passed style.
        @descr  Searches for style XFs only. */
    bool                Equals( const SfxStyleSheetBase& rStyleSheet ) const;

    /** Sets the resulting Excel palette index from all used color IDs (border and area). */
    void                SetFinalColors();

    /** Returns true, if this XF record is completely equal to the passed. */
    bool                Equals( const XclExpXF& rCmpXF ) const;

protected:
    explicit            XclExpXF( const XclExpRoot& rRoot, bool bCellXF );

protected:  // access for XclExpDefaultXF
    const SfxItemSet*   mpItemSet;          /// Pointer to the item set (we do not own it).

    XclExpCellProt      maProtection;       /// Cell protection flags.
    XclExpCellAlign     maAlignment;        /// All alignment attributes.
    XclExpCellBorder    maBorder;           /// Border line style.
    XclExpCellArea      maArea;             /// Background area style.
    sal_uInt32          mnParentXFId;       /// XF ID of parent XF record.
    ULONG               mnScNumFmt;         /// Calc number format index.
    sal_uInt16          mnXclFont;          /// Excel font index.
    sal_uInt16          mnXclNumFmt;        /// Excel number format index.

private:
    using               XclXFBase::Equals;

    /** Initializes with default values. */
    void                InitDefault();
    /** Fills all members from the passed item set.
        @param bDefStyle  true = This is the "Default"/"Normal" style - needs special handling. */
    void                Init(
                            const SfxItemSet& rItemSet,
                            sal_Int16 nScript,
                            ULONG nForceScNumFmt,
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

    /** Sets the parent XF ID. Only allowed for cell XFs. */
    void                SetParent( sal_uInt32 nParentXFId );

    /** Sets all "attribute used" flags explicitely.
        @descr  The following Set***() functions set the appropriate flag too. */
    void                SetUsedFlags(
                            bool bProtUsed, bool bFontUsed, bool bFmtUsed,
                            bool bAlignUsed, bool bBorderUsed, bool bAreaUsed );
    /** Sets the cell protection flags. */
    void                SetProtection( const XclExpCellProt& rProtection );
    /** Sets the Excel font index. */
    void                SetFont( sal_uInt16 nXclFont );
    /** Sets the Excel number format index. */
    void                SetNumFmt( sal_uInt16 nXclNumFmt );
    /** Sets cell alignment attributes. */
    void                SetAlignment( const XclExpCellAlign& rAlignment );
    /** Sets a cell border style. */
    void                SetBorder( const XclExpCellBorder& rBorder );
    /** Sets a cell area style. */
    void                SetArea( const XclExpCellArea& rArea );
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
        @return  A unique XF record ID. */
    sal_uInt32          InsertWithNumFmt(
                            const ScPatternAttr* pPattern, sal_Int16 nScript,
                            ULONG nForceScNumFmt );
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

    /** Writes all XF records contained in this buffer. */
    virtual void        Save( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpXF >    XclExpXFList;
    typedef XclExpXFList::RecordRefType     XclExpXFRef;
    typedef XclExpRecordList< XclExpStyle > XclExpStyleList;

private:
    /** Returns the XF ID of the cell XF containing the passed format. */
    sal_uInt32          FindXF( const ScPatternAttr& rPattern, ULONG nForceScNumFmt,
                            sal_uInt16 nForceXclFont, bool bForceLineBreak ) const;
    /** Returns the XF ID of the style XF containing the passed style. */
    sal_uInt32          FindXF( const SfxStyleSheetBase& rStyleSheet ) const;

    /** Returns the XF ID of a built-in style XF, searches by style identifier. */
    sal_uInt32          FindBuiltInXF( sal_uInt8 nStyleId, sal_uInt8 nLevel = EXC_STYLE_NOLEVEL ) const;

    /** Tries to find the XF record containing the passed format or inserts a new record.
        @return  The XF record ID. */
    sal_uInt32          InsertCellXF( const ScPatternAttr* pPattern, sal_Int16 nScript,
                            ULONG nForceScNumFmt,
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

    XclExpXFList        maXFList;           /// List of all XF records.
    XclExpStyleList     maStyleList;        /// List of all STYLE records.
    XclExpBuiltInMap    maBuiltInMap;       /// Contained elements describe built-in XFs.
    ScfUInt16Vec        maXFIndexVec;       /// Maps XF IDs to XF indexes.
    XclExpXFList        maSortedXFList;     /// List of XF records in XF index order.
};

// ============================================================================

#endif

