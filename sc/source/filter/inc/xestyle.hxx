/*************************************************************************
 *
 *  $RCSfile: xestyle.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:41:22 $
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

#ifndef SC_XESTYLE_HXX
#define SC_XESTYLE_HXX

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
- Buffers for style records (PALETTE, FONT, FORMAT, XF).
- Page styles export.
============================================================================ */

// PALETTE record - color information =========================================

/** Different types of colors in a document. */
enum XclColorType
{
    xlColorCellText,        /// Text in a cell.
    xlColorCellBorder,      /// Border of a cell.
    xlColorCellArea,        /// Background area of a cell.
    xlColorChartText,       /// Text color in a chart.
    xlColorChartLine,       /// Line in a chart.
    xlColorChartArea,       /// Area in a chart.
    xlColorGrid             /// Spreadsheet grid color.
};


// ----------------------------------------------------------------------------

/** Stores all used colors in the document.
    @descr  Supports color reduction to the maximum count of the current BIFF version.
    An instance of this class collects all colors in the conversion phase of the export,
    using the InsertColor() function. It returns a unique identidier for each passed color.
    After the entire document is converted, the palette will be reduced to the number of
    colors supported by the current BIFF version, using the function Reduce().
    Then, in the streaming phase, the functions GetColorIndex() and GetMixedColors()
    return the real Excel palette index for all color identifiers. */
class XclExpPalette : public XclExpRecord, public XclDefaultPalette, protected XclExpRoot
{
public:
    explicit                    XclExpPalette( const XclExpRoot& rRoot );

    /** Inserts the color into the list and updates weighting.
        @param nAutoDefault  The Excel palette index for automatic color.
        @return  A unique ID for this color. */
    sal_uInt32                  InsertColor( const Color& rColor, XclColorType eType, sal_uInt16 nAutoDefault = 0 );
    /** Returns the color ID representing a fixed Excel palette index (i.e. for auto colors). */
    static sal_uInt32           GetColorIdFromIndex( sal_uInt16 nIndex );

    /** Reduces the color list to the maximum count of the current BIFF version. */
    void                        Reduce();

    /** Returns the Excel palette index of the color with passed color ID. */
    sal_uInt16                  GetColorIndex( sal_uInt32 nColorId ) const;
    /** Returns the Excel palette index of the passed color (searches for nearest color).
        @param nAutoDefault  The Excel palette index for automatic color. */
    sal_uInt16                  GetColorIndex( const Color& rColor, sal_uInt16 nAutoDefault = 0 ) const;

    /** Returns a foreground and background color for the two passed color IDs.
        @descr  If rnXclPattern contains a solid pattern, this function tries to find
        the two best fitting colors and a mix pattern (25%, 50% or 75%) for nForeColorId.
        This will result in a better approximation to the passed foreground color. */
    void                        GetMixedColors(
                                    sal_uInt16& rnXclForeIx, sal_uInt16& rnXclBackIx, sal_uInt8& rnXclPattern,
                                    sal_uInt32 nForeColorId, sal_uInt32 nBackColorId ) const;

    /** Returns the RGB color data for a (non-zero-based) Excel palette entry.
        @return  The color from current or default palette or COL_AUTO, if nothing else found. */
    ColorData                   GetColorData( sal_uInt16 nXclIndex ) const;
    /** Returns the color for a (non-zero-based) Excel palette entry.
        @return  The color from current or default palette or COL_AUTO, if nothing else found. */
    inline Color                GetColor( sal_uInt16 nXclIndex ) const
                                    { return Color( GetColorData( nXclIndex ) ); }


private:
    class XclListColor;

    /** Returns the Excel index of a 0-based color index. */
    inline sal_uInt16           GetXclIndex( sal_uInt32 nIndex ) const;

    /** Returns the original inserted color represented by the color ID nColorId. */
    const Color&                GetOriginalColor( sal_uInt32 nColorId ) const;

    /** Returns the ordered insertion index for rColor in rnIndex.
        @param rbIsEqual  Returns true, if the color already exists. */
    void                        SearchListEntry(
                                    sal_uInt32& rnIndex, bool& rbIsEqual,
                                    const Color& rColor ) const;
    /** Creates and inserts a new color list entry at the specified list position. */
    XclListColor*               CreateListEntry( const Color& rColor, sal_uInt32 nIndex );
    /** Merges two colors and removes the color specified by nRemove. */
    void                        MergeListColors( sal_uInt32 nKeep, sal_uInt32 nRemove );

    /** Finds the least used color and returns its current list index. */
    sal_uInt32                  GetLeastUsedListColor() const;
    /** Returns the list index of the color nearest to rColor.
        @param nIgnore  List index of a color which will be ignored.
        @return  The list index of the found color. */
    sal_uInt32                  GetNearestListColor( const Color& rColor, sal_uInt32 nIgnore ) const;
    /** Returns the list index of the color nearest to the color with list index nIndex. */
    sal_uInt32                  GetNearestListColor( sal_uInt32 nIndex ) const;

    /** Returns in rnIndex the palette index of the color nearest to rColor.
        @param bDefaultOnly  true = Searches for default colors only (colors never replaced).
        @return  The distance from passed color to found color. */
    sal_Int32                   GetNearestPaletteColor(
                                    sal_uInt32& rnIndex,
                                    const Color& rColor, bool bDefaultOnly ) const;
    /** Returns in rnFirst and rnSecond the palette indexes of the two colors nearest to rColor.
        @return  The minimum distance from passed color to found colors. */
    sal_Int32                   GetNearPaletteColors(
                                    sal_uInt32& rnFirst, sal_uInt32& rnSecond,
                                    const Color& rColor ) const;

    /** Writes the contents of the PALETTE record. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    /** Represents an entry in a color list.
        @descr  the color stores a weighting value, which increases the more the color
        is used in the document. Heavy-weighted colors will change less than others on
        color reduction. */
    class XclListColor
    {
        DECL_FIXEDMEMPOOL_NEWDEL( XclListColor )

    private:
        Color                   maColor;    /// The color value of this palette entry.
        sal_uInt32              mnColorId;  /// Unique color ID for color reduction.
        sal_uInt32              mnWeight;   /// Weighting for color reduction.

    public:
        explicit                XclListColor( const Color& rColor, sal_uInt32 nColorId );

        /** Returns the RGB color value of the color. */
        inline const Color&     GetColor() const { return maColor; }
        /** Returns the unique ID of the color. */
        inline sal_uInt32       GetColorId() const { return mnColorId; }
        /** Returns the current weighting of the color. */
        inline sal_uInt32       GetWeighting() const { return mnWeight; }
        /** Returns true, if this color value is greater than rColor (simple arithmetic comparison). */
        inline bool             IsGreater( const Color& rColor ) const { return maColor.GetColor() > rColor.GetColor(); }
        /** Returns true, if this color is equal to rColor. */
        inline bool             IsEqual( const Color& rColor ) const { return maColor.GetColor() == rColor.GetColor(); }

        /** Updates the weighting of this color dependent from nColorType. */
        void                    UpdateWeighting( XclColorType eType );
        /** Adds the weighting of rColor to this color. */
        inline void             AddWeighting( const XclListColor& rColor ) { mnWeight += rColor.mnWeight; }
        /** Merges this color with rColor, regarding weighting settings. */
        void                    Merge( const XclListColor& rColor );
    };

    /** Data for each inserted original color, represented by a color ID. */
    struct XclColorIdData
    {
        Color                   maColor;    /// The original inserted color.
        sal_uInt32              mnIndex;    /// Maps current color ID to color list or export color vector.
        /** Sets the contents of this struct. */
        inline void             Set( const Color& rColor, sal_uInt32 nIndex ) { maColor = rColor; mnIndex = nIndex; }
    };

    /** A color that will be written to the Excel file. */
    struct XclPaletteColor
    {
        Color                   maColor;    /// Resulting color to export.
        bool                    mbDefault;  /// true = Entry is a default color.
        /** Sets a new color.
            @param bDefault  true = Color is a color from default palette. */
        inline void             SetColor( const Color& rColor, bool bDefault = false ) { maColor = rColor; mbDefault = bDefault; }
    };

    typedef ScfDelList< XclListColor >          XclListColorList;
    typedef ::std::vector< XclColorIdData >     XclColorIdDataVec;
    typedef ::std::vector< XclPaletteColor >    XclPaletteColorVec;

    XclListColorList            maColorList;        /// Working color list.
    XclColorIdDataVec           maColorIdDataVec;   /// Data of all CIDs.
    XclPaletteColorVec          maPalette;          /// Contains resulting colors to export.
    sal_uInt32                  mnLastIx;           /// Last insertion index for search opt.
};

inline sal_uInt16 XclExpPalette::GetXclIndex( sal_uInt32 nIndex ) const
{
    return static_cast< sal_uInt16 >( nIndex + EXC_COLOR_USEROFFSET );
}


// FONT record - font information =============================================

class Font;
class SvxFont;

/** Stores all data of an Excel font and provides export of FONT records. */
class XclExpFont : public XclExpRecord, protected XclExpRoot
{
public:
    explicit                    XclExpFont( const XclExpRoot& rRoot, const Font& rFont );
    explicit                    XclExpFont( const XclExpRoot& rRoot, const SvxFont& rFont );
    explicit                    XclExpFont( const XclExpRoot& rRoot, const XclFontData& rFontData );

    /** Returns read-only access to font data. */
    inline const XclFontData&   GetFontData() const { return maData; }
    /** Compares this font with the passed font.
        @descr  Ignores color settings, if no color is set. */
    bool                        Equals( const XclExpFont& rCmpFont ) const;

    /** Returns true, if this font contains a specific color (if SetColor was called before). */
    inline bool                 HasColor() const { return mbHasColor; }
    /** Returns the unique color ID of the font color from palette. */
    inline sal_uInt32           GetColorId() const { return mnColorId; }
    /** Sets the color ID for this font. */
    void                        SetColorId( sal_uInt32 nColorId );

private:
    /** Calculates an internal hash value for faster comparison. */
    void                        CalcHash();

    /** Sets the ColorId, internal hash value and the record size. */
    void                        SetColorHashAndRecSize( const Font& rFont );

    /** Writes the contents of the FONT record. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    XclFontData                 maData;         /// All font attributes.
    sal_uInt32                  mnColorId;      /// Unique color ID for text color.
    sal_uInt32                  mnHash;         /// Hash value for fast comparison.
    bool                        mbHasColor;     /// false = Font does not use a color (i.e. chart).
};


// ----------------------------------------------------------------------------

class ScPatternAttr;

/** Stores the data of all fonts used in the document. */
class XclExpFontBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit                    XclExpFontBuffer( const XclExpRoot& rRoot );

    /** Returns the specified font from font list. */
    inline const XclExpFont*    GetFont( sal_uInt32 nIndex ) const
                                    { return maFontList.GetObject( nIndex ); }

    /** Searches for the passed font and returns its Excel font index.
        @return  Excel font index or nDefault, if not found. */
    sal_uInt16                  GetIndex( const XclExpFont& rFont, sal_uInt16 nDefault = EXC_FONT_APP );

    /** Inserts the font into the buffer if not present, otherwise deletes it.
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16                  Insert( XclExpFont*& rpFont, bool bAppFont = false );
    /** Inserts the font into the buffer if not present.
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16                  Insert( const Font& rFont, bool bAppFont = false );
    /** Inserts the SvxFont into the buffer if not present i.e. where escapements are used
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16                  Insert( const SvxFont& rFont, bool bAppFont = false );
    /** Inserts the font contained in the passed item set into the buffer, if not present.
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16                  Insert( const SfxItemSet& rItemSet, bool bAppFont = false );
    /** Inserts the font contained in rPattern into the buffer if not present.
        @param bAppFont  true = Sets the application font; false = Inserts a new font.
        @return  The resulting Excel font index. */
    sal_uInt16                  Insert( const ScPatternAttr& rPattern, bool bAppFont = false );

    /** Writes all FONT records contained in this buffer. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Initializes the default fonts for the current BIFF version. */
    void                        InitDefaultFonts();
    /** Converts a list index into Excel font index. */
    sal_uInt16                  GetXclIndex( sal_uInt32 nIndex ) const;
    /** Tries to find the passed font and returns the current list index. */
    sal_uInt32                  Find( const XclExpFont& rFont );

private:
    typedef XclExpRecordList< XclExpFont > XclExpFontList;

    XclExpFontList              maFontList;     /// List of all FONT records.
    XclFontData                 maAppFont;      /// Application font (for column width).
    sal_uInt32                  mnXclMaxCount;  /// Maximum number of fonts.
};


// FORMAT record - number formats =============================================

/** Stores a core number format index with corresponding Excel format index. */
struct XclExpNumFmt
{
    sal_uInt32                  mnFormatIx;     /// Core index of the number format.
    sal_uInt16                  mnXclIx;        /// Resulting Excel format index.

    inline explicit             XclExpNumFmt( sal_uInt32 nFormatIx, sal_uInt16 nXclIx ) :
                                    mnFormatIx( nFormatIx ), mnXclIx( nXclIx ) {}
};


// ----------------------------------------------------------------------------

class SvNumberFormatter;

/** Stores all number formats used in the document. */
class XclExpNumFmtBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit                    XclExpNumFmtBuffer( const XclExpRoot& rRoot );

    virtual                     ~XclExpNumFmtBuffer();

    /** Returns the core index of the current standard number format. */
    inline sal_uInt32           GetStandardFormat() const { return mnStdFmt; }

    /** Inserts a number format into the format buffer.
        @param nFormatIx  The core index of the number format.
        @return  The resulting Excel format index. */
    sal_uInt16                  Insert( sal_uInt32 nScFormatIx );

    /** Writes all FORMAT records contained in this buffer. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Writes the FORMAT record with index nXclIx and format string rFormatStr. */
    void                        WriteFormatRecord( XclExpStream& rStrm, sal_uInt16 nXclIx, const String& rFormatStr );
    /** Writes the FORMAT record represented by rFormat. */
    void                        WriteFormatRecord( XclExpStream& rStrm, const XclExpNumFmt& rFormat );

private:
    typedef ::std::auto_ptr< SvNumberFormatter >    SvNumberFormatterPtr;
    typedef NfKeywordTable*                         NfKeywordTablePtr;
    typedef ::std::vector< XclExpNumFmt >           XclExpNumFmtVec;

    SvNumberFormatterPtr        mpFormatter;    /// Special number formatter for conversion.
    NfKeywordTablePtr           mpKeywordTable; /// Replacement table.
    XclExpNumFmtVec             maFormatMap;    /// Maps core formats to Excel indexes.
    sal_uInt32                  mnStdFmt;       /// Key for standard number format.
    sal_uInt16                  mnXclOffset;    /// Offset to first user defined format.
};


// XF, STYLE record - Cell formatting =========================================

/** Extends the XclCellProt struct for export.
    @descr  Provides functions to fill from item sets and to fill to Excel record data. */
struct XclExpCellProt : public XclCellProt
{
    /** Fills the protection attributes from the passed item set.
        @descr  Fills only the attributes exported in the passed BIFF version.
        @return  true = At least one protection item is set. */
    bool                        FillFromItemSet( const SfxItemSet& rItemSet, XclBiff eBiff, bool bStyle = false );

#if 0
    /** Fills the data to the passed fields of a BIFF2 XF record. */
    void                        FillToXF2( sal_uInt8& rnNumFmt ) const;
#endif
    /** Fills the data to the passed fields of a BIFF3-BIFF8 XF record. */
    void                        FillToXF3( sal_uInt16& rnProt ) const;
};


// ----------------------------------------------------------------------------

/** Extends the XclCellAlign struct for export.
    @descr  Provides functions to fill from item sets and to fill to Excel record data. */
struct XclExpCellAlign : public XclCellAlign
{
    /** Fills the alignment attributes from the passed item set.
        @descr  Fills only the attributes exported in the passed BIFF version.
        @param bForceWrapped  true = Set text wrap flag unconditionally.
        @return  true = At least one alignment item is set. */
    bool                        FillFromItemSet(
                                    const SfxItemSet& rItemSet, XclBiff eBiff,
                                    bool bForceWrapped = false, bool bStyle = false );

#if 0
    /** Fills the data to the passed fields of a BIFF2 XF record. */
    void                        FillToXF2( sal_uInt8& rnFlags ) const;
    /** Fills the data to the passed fields of a BIFF3 XF record. */
    void                        FillToXF3( sal_uInt16& rnAlign ) const;
    /** Fills the data to the passed fields of a BIFF4 XF record. */
    void                        FillToXF4( sal_uInt16& rnAlign ) const;
#endif
    /** Fills the data to the passed fields of a BIFF5/BIFF7 XF record. */
    void                        FillToXF5( sal_uInt16& rnAlign ) const;
    /** Fills the data to the passed fields of a BIFF8 XF record. */
    void                        FillToXF8( sal_uInt16& rnAlign, sal_uInt16& rnMiscAttrib ) const;
};


// ----------------------------------------------------------------------------

/** Extends the XclCellBorder struct for export.
    @descr  Provides functions to fill from item sets and to fill to Excel record data. */
struct XclExpCellBorder : public XclCellBorder
{
    sal_uInt32                  mnLeftColorId;      /// Color ID for left line.
    sal_uInt32                  mnRightColorId;     /// Color ID for right line.
    sal_uInt32                  mnTopColorId;       /// Color ID for top line.
    sal_uInt32                  mnBottomColorId;    /// Color ID for bottom line.

    explicit                    XclExpCellBorder();

    /** Fills the border attributes from the passed item set.
        @descr  Fills only the attributes exported in the passed BIFF version.
        @return  true = At least one border item is set. */
    bool                        FillFromItemSet(
                                    const SfxItemSet& rItemSet, XclExpPalette& rPalette,
                                    XclBiff eBiff, bool bStyle = false );
    /** Fills the mn***Color base members from the mn***ColorId members. */
    void                        SetFinalColors( const XclExpPalette& rPalette );

#if 0
    /** Fills the data to the passed fields of a BIFF2 XF record. */
    void                        FillToXF2( sal_uInt8& rnFlags ) const;
    /** Fills the data to the passed fields of a BIFF3/BIFF4 XF record. */
    void                        FillToXF3( sal_uInt32& rnBorder ) const;
#endif
    /** Fills the data to the passed fields of a BIFF5/BIFF7 XF record. */
    void                        FillToXF5( sal_uInt32& rnBorder, sal_uInt32& rnArea ) const;
    /** Fills the data to the passed fields of a BIFF8 XF record. */
    void                        FillToXF8( sal_uInt32& rnBorder1, sal_uInt32& rnBorder2 ) const;

    /** Fills the data to the passed fields of a BIFF8 CF (conditional format) record. */
    void                        FillToCF8( sal_uInt16& rnLine, sal_uInt32& rnColor ) const;
};


// ----------------------------------------------------------------------------

/** Extends the XclCellArea struct for export.
    @descr  Provides functions to fill from item sets and to fill to Excel record data. */
struct XclExpCellArea : public XclCellArea
{
    sal_uInt32                  mnForeColorId;  /// Foreground color ID.
    sal_uInt32                  mnBackColorId;  /// Background color ID.

    explicit                    XclExpCellArea();

    /** Fills the area attributes from the passed item set.
        @descr  Fills only the attributes exported in the passed BIFF version.
        @return  true = At least one area item is set. */
    bool                        FillFromItemSet(
                                    const SfxItemSet& rItemSet, XclExpPalette& rPalette,
                                    XclBiff eBiff, bool bStyle = false );
    /** Fills the mn***Color base members from the mn***ColorId members. */
    void                        SetFinalColors( const XclExpPalette& rPalette );

#if 0
    /** Fills the data to the passed fields of a BIFF2 XF record. */
    void                        FillToXF2( sal_uInt8& rnFlags ) const;
    /** Fills the data to the passed fields of a BIFF3/BIFF4 XF record. */
    void                        FillToXF3( sal_uInt16& rnArea ) const;
#endif
    /** Fills the data to the passed fields of a BIFF5/BIFF7 XF record. */
    void                        FillToXF5( sal_uInt32& rnArea ) const;
    /** Fills the data to the passed fields of a BIFF8 XF record. */
    void                        FillToXF8( sal_uInt32& rnBorder2, sal_uInt16& rnArea ) const;

    /** Fills the data to the passed fields of a BIFF8 CF (conditional format) record. */
    void                        FillToCF8( sal_uInt16& rnPattern, sal_uInt16& rnColor ) const;
};


// ----------------------------------------------------------------------------

class SfxStyleSheetBase;

/** Represents an XF record which contains all formatting data of a cell or cell style. */
class XclExpXF : public XclXFBase, public XclExpRecord, protected XclExpRoot
{
public:
    /** Constructs a cell XF record from the passed Calc cell formatting.
        @param nForcedNumFmt  If not set to NUMBERFORMAT_ENTRY_NOT_FOUND, it will overwrite
        the number format of the passed item set.
        @param bForceWrapped  true = set text wrap flag unconditionally. */
    explicit                    XclExpXF(
                                    const XclExpRoot& rRoot,
                                    const ScPatternAttr& rPattern,
                                    sal_uInt32 nForcedNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND,
                                    bool bForceWrapped = false,
                                    sal_uInt16 nFontIx = EXC_FONT_NOTFOUND);
    /** Constructs a style XF record from the passed cell style sheet. */
    explicit                    XclExpXF(
                                    const XclExpRoot& rRoot,
                                    const SfxStyleSheetBase& rStyleSheet );

    /** Returns true, if this XF record represents the passed cell formatting.
        @descr  Searches for cell XFs only. */
    bool                        Equals(
                                    const ScPatternAttr& rPattern,
                                    sal_uInt32 nForcedNumFmt,
                                    bool bForceWrapped,
                    sal_uInt16 nFontIx ) const;

    /** Returns true, if this XF record represents the passed style.
        @descr  Searches for style XFs only. */
    bool                        Equals( const SfxStyleSheetBase& rStyleSheet ) const;

    /** Sets the resulting Excel palette index from all used color IDs (border and area). */
    void                        SetFinalColors();

    /** Returns true, if this XF record is completely equal to the passed. */
    bool                        Equals( const XclExpXF& rCmpXF ) const;

protected:
    explicit                    XclExpXF( const XclExpRoot& rRoot, bool bCellXF );

protected:  // access for XclExpDefaultXF
    const SfxItemSet*           mpItemSet;          /// Pointer to the item set (we do not own it).

    XclExpCellProt              maProtection;       /// Cell protection flags.
    XclExpCellAlign             maAlignment;        /// All alignment attributes.
    XclExpCellBorder            maBorder;           /// Border line style.
    XclExpCellArea              maArea;             /// Background area style.
    sal_uInt32                  mnParentXFId;       /// XF ID of parent XF record.
    sal_uInt16                  mnFont;             /// Excel font index.
    sal_uInt16                  mnNumFmt;           /// Excel number format index.

private:
    /** Initializes with default values. */
    void                        InitDefault();
    /** Fills all members from the passed item set.
        @param bDefStyle  true = This is the "Default"/"Normal" style - needs special handling. */
    void                        Init(
                                    const SfxItemSet& rItemSet,
                                    sal_uInt32 nForcedNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND,
                                    bool bForceWrapped = false,
                                    sal_uInt16 nFontIx = EXC_FONT_NOTFOUND,
                                    bool bDefStyle = false);

    /** Returns the bits specifying the used attributes.
        @descr  In cell XFs a set bit means a used attribute, in style XF a cleared
        bit means a used attribute. This method regards the cell/style state.
        @return  The mask based on bit 0 (not yet bit-shifted as needed for export). */
    sal_uInt8                   GetUsedFlags() const;

    void                        WriteBody5( XclExpStream& rStrm );
    void                        WriteBody8( XclExpStream& rStrm );

    /** Writes the contents of the XF record. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

/** Represents a default XF record. Supports methods to set attributes directly. */
class XclExpDefaultXF : public XclExpXF
{
public:
    explicit                    XclExpDefaultXF( const XclExpRoot& rRoot, bool bCellXF );

    /** Sets the parent XF ID. Only allowed for cell XFs. */
    void                        SetParent( sal_uInt32 nParentXFId );

    /** Sets all "attribute used" flags explicitely.
        @descr  The following Set***() functions set the appropriate flag too. */
    void                        SetUsedFlags(
                                    bool bProtUsed, bool bFontUsed, bool bFmtUsed,
                                    bool bAlignUsed, bool bBorderUsed, bool bAreaUsed );
    /** Sets the cell protection flags. */
    void                        SetProtection( const XclExpCellProt& rProtection );
    /** Sets the Excel font index. */
    void                        SetFont( sal_uInt16 nFont );
    /** Sets the Excel number format index. */
    void                        SetNumFmt( sal_uInt16 nNumFmt );
    /** Sets cell alignment attributes. */
    void                        SetAlignment( const XclExpCellAlign& rAlignment );
    /** Sets a cell border style. */
    void                        SetBorder( const XclExpCellBorder& rBorder );
    /** Sets a cell area style. */
    void                        SetArea( const XclExpCellArea& rArea );
};


// ----------------------------------------------------------------------------

/** Represents a STYLE record containing the data of a cell style.
    @descr  The calss is able to store built-in and user-defined styles. */
class XclExpStyle : public XclExpRecord
{
public:
    explicit                    XclExpStyle( sal_uInt32 nXFId, const String& rStyleName );
    explicit                    XclExpStyle( sal_uInt32 nXFId, sal_uInt8 nStyleId, sal_uInt8 nLevel = EXC_STYLE_NOLEVEL );

    /** Returns the index of the XF record used by this style. */
    inline sal_uInt32           GetXFId() const { return mnXFId; }
    /** Returns true, if this record represents an Excel built-in style. */
    inline bool                 IsBuiltIn() const { return mnStyleId != EXC_STYLE_USERDEF; }

private:
    /** Writes the contents of the STYLE record. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    String                      maName;         /// Name of the cell style.
    sal_uInt32                  mnXFId;         /// XF record ID with style formatting.
    sal_uInt8                   mnStyleId;      /// Built-in style identifier.
    sal_uInt8                   mnLevel;        /// Outline level for RowLevel and ColLevel styles.
};


// ----------------------------------------------------------------------------

/** Stores all XF records (cell formats and cell styles) in the document.
    @descr  Stores also the names of user defined cell styles (STYLE records).
    Supports reduction to the maximum count of XF records of the current BIFF version.
    An instance of this class collects all XF records in the conversion phase of the export,
    using the Insert() and InsertStyle() functions. It returns a unique identidier for each
    XF record. After the entire document is converted, the list will be reduced to the
    number of XF records supported by the current BIFF version, using the function Reduce().
    Then, in the streaming phase, the function GetXFIndex() returns the real Excel XF index
    for all XF identifiers. */
class XclExpXFBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit                    XclExpXFBuffer( const XclExpRoot& rRoot );

    /** Inserts predefined built-in styles and user-defined styles. */
    void                        InitDefaults();

    /** Finds or creates a cell XF record for the passed item set.
        @param bForceWrapped  true = cell contains hard newlines.
        In this case the text wrap flag must be set in the XF record.
        @return  A unique XF record ID. */
    sal_uInt32                  Insert( const ScPatternAttr* pPattern, bool bForceWrapped = false, sal_uInt16 nFontIx = EXC_FONT_NOTFOUND );
    /** Finds or creates a cell XF record for the passed item set, with custom number format.
        @param nForcedNumFmt  The number format to be exported, i.e. formula result type.
        This format will always overwrite the cell's number format.
        @return  A unique XF record ID. */
    sal_uInt32                  Insert( const ScPatternAttr* pPattern, sal_uInt32 nForcedNumFmt );
    /** Inserts the passed cell style. Creates a style XF record and a STYLE record.
        @return  A unique XF record ID. */
    sal_uInt32                  InsertStyle( const SfxStyleSheetBase* pStyleSheet );
    /** Returns the XF ID representing a fixed Excel XF index (i.e. for built-in XFs). */
    static sal_uInt32           GetXFIdFromIndex( sal_uInt16 nXFIndex );

    /** Reduces the XF record list to the maximum allowed number of records. */
    void                        Reduce();

    /** Returns the Excel XF index of the XF record with passed XF ID. */
    sal_uInt16                  GetXFIndex( sal_uInt32 nXFId ) const;

    /** Writes all XF records contained in this buffer. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Returns the XF ID of the cell XF containing the passed format. */
    sal_uInt32                  FindXF(
                                    const ScPatternAttr& rPattern,
                                    sal_uInt32 nForcedNumFmt, bool bForceWrapped, sal_uInt16 nFontIx ) const;
    /** Returns the XF ID of the style XF containing the passed style. */
    sal_uInt32                  FindXF( const SfxStyleSheetBase& rStyleSheet ) const;

    /** Returns the style list index of the STYLE record containing the passed XF ID. */
    sal_uInt32                  FindStyle( sal_uInt32 nXFId ) const;

    /** Tries to find the XF record containing the passed format or inserts a new record.
        @return  The XF record ID. */
    sal_uInt32                  InsertCellXF(
                                    const ScPatternAttr* pPattern,
                                    sal_uInt32 nForcedNumFmt, bool bForceWrapped, sal_uInt16 nFontIx );
    /** Inserts the passed cell style. Creates a style XF record and a STYLE record.
        @return  The XF record ID. */
    sal_uInt32                  InsertStyleXF( const SfxStyleSheetBase& rStyleSheet );

    /** Inserts an XF and a STYLE record for all user defined style sheets. */
    void                        InsertUserStyles();

    /** Inserts a default XF and STYLE record. */
    void                        InsertDefaultStyle( XclExpXF* pXF, sal_uInt8 nStyleId, sal_uInt8 nLevel = 0xFF );
    /** Inserts all default XF and STYLE records. */
    void                        InsertDefaultRecords();

    /** Appends a XF index to the internal ID<->index maps. */
    void                        AppendXFIndex( sal_uInt32 nXFId );

private:
    typedef ScfDelList< XclExpXF >          XclExpXFList;
    typedef XclExpRecordList< XclExpStyle > XclExpStyleList;
    typedef ::std::vector< bool >           BoolVec;
    typedef ::std::vector< sal_uInt16 >     XclExpXFIndexVec;
    typedef ::std::vector< XclExpXF* >      XclExpXFPtrVec;

    XclExpXFList                maXFList;       /// List of all XF records.
    XclExpStyleList             maStyleList;    /// List of all STYLE records.
    BoolVec                     maPredefined;   /// true = Corresponding XF still predefined.
    XclExpXFIndexVec            maXFIndexVec;   /// Maps XF IDs to XF indexes.
    XclExpXFPtrVec              maXFPtrVec;     /// Pointer to XF records in resulting export order.
};


// ============================================================================

#endif

