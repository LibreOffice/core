/*************************************************************************
 *
 *  $RCSfile: xestyle.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:10 $
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
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_NFKEYTAB_HXX
#include <svtools/nfkeytab.hxx>
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
    colors supported by the current BIFF version, using the function ReduceColors().
    Then, in the streaming phase, the functions GetColorIndex() and GetMixedColors()
    return the real Excel palette index for all color identifiers. */
class XclExpPalette : public XclExpRecord, public XclDefaultPalette, protected XclExpRoot
{
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

public:
    explicit                    XclExpPalette( const XclExpRoot& rRoot );

    /** Activates the default colors for the current BIFF version. */
    void                        OnChangeBiff();

    /** Inserts the color into the list and updates weighting.
        @param nAutoDefault  The Excel palette index for automatic color.
        @return  A unique ID for this color. */
    sal_uInt32                  InsertColor( const Color& rColor, XclColorType eType, sal_uInt16 nAutoDefault = 0 );
    /** Inserts the Excel color index into the list.
        @return  A unique ID, that will later return exactly the passed index. */
    sal_uInt32                  InsertIndex( sal_uInt16 nIndex );

    /** Reduces the color list to the maximum count of the current BIFF version. */
    void                        ReduceColors();

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
        @param nDefault  Is returned, if nothing else could be found. */
    ColorData                   GetColorData( sal_uInt16 nXclIndex, ColorData nDefault = COL_AUTO ) const;
    /** Returns the color for a (non-zero-based) Excel palette entry.
        @param nDefault  Is returned, if nothing else could be found. */
    inline Color                GetColor( sal_uInt16 nXclIndex, ColorData nDefault = COL_AUTO ) const;

private:
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
};

inline Color XclExpPalette::GetColor( sal_uInt16 nXclIndex, ColorData nDefault ) const
{
    return Color( GetColorData( nXclIndex, nDefault ) );
}


// FONT record - font information =============================================

class Font;

/** Stores all data of an Excel font and provides export of FONT records. */
class XclExpFont : public XclExpRecord, protected XclExpRoot
{
private:
    XclFontData                 maData;         /// All font attributes.
    sal_uInt32                  mnColorId;      /// Unique color ID for text color.
    sal_uInt32                  mnHash;         /// Hash value for fast comparison.
    bool                        mbHasColor;     /// false = Font does not use a color (i.e. chart).

public:
    explicit                    XclExpFont( const XclExpRoot& rRoot );

    /** Returns read-only access to font data. */
    inline const XclFontData&   GetFontData() const { return maData; }

    void                        SetName( const String& rName );
    inline void                 SetFamily( FontFamily eFamily ) { maData.mnFamily = GetXclFamily( eFamily ); }
    inline void                 SetCharSet( rtl_TextEncoding eCharSet ) { maData.mnCharSet = GetXclCharSet( eCharSet ); }
    void                        SetColor( const Color& rColor );
    void                        SetColorId( sal_uInt32 nColorId );
    void                        SetHeight( sal_Int32 nTwips );
    inline void                 SetWeight( FontWeight eWeight ) { maData.mnWeight = GetXclWeight( eWeight ); }
    inline void                 SetUnderline( FontUnderline eUnderl ) { maData.meUnderline = GetXclUnderline( eUnderl ); }
    inline void                 SetEscapement( SvxEscapement eEscapem ) { maData.meEscapem = GetXclEscapement( eEscapem ); }
    inline void                 SetItalic( bool bItalic )       { maData.mbItalic = bItalic; }
    inline void                 SetStrikeout( bool bStrikeout ) { maData.mbStrikeout = bStrikeout; }
    inline void                 SetOutline( bool bOutline )     { maData.mbOutline = bOutline; }
    inline void                 SetShadow( bool bShadow )       { maData.mbShadow = bShadow; }

    /** Initializes this record with the passed font data. */
    void                        SetFont( const Font& rFont );

    /** Calculates an internal hash value for faster comparison. */
    void                        CalcHash();

    /** Compares this font with the passed font.
        @descr  Ignores color settings, if no color is set. */
    bool                        operator==( const XclExpFont& rCmp ) const;

    /** Returns true, if this font contains a specific color (if SetColor was called before). */
    inline bool                 HasColor() const { return mbHasColor; }
    /** Returns the unique color ID of the font color from palette. */
    inline sal_uInt32           GetColorId() const { return mnColorId; }

    /** Converts a font family to Excel font family. */
    static sal_uInt8            GetXclFamily( FontFamily eFamily );
    /** Converts a text encoding to Excel character set. */
    static sal_uInt8            GetXclCharSet( rtl_TextEncoding eCharSet );
    /** Converts a font weight to Excel font weight. */
    static sal_uInt16           GetXclWeight( FontWeight eWeight );
    /** Converts a font underline style to Excel font underline style. */
    static XclUnderline         GetXclUnderline( FontUnderline eUnderl );
    /** Converts a font escapement to Excel font escapement. */
    static XclEscapement        GetXclEscapement( SvxEscapement eEscapem );

private:
    /** Writes the contents of the FONT record. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

class ScPatternAttr;

/** Stores the data of all fonts used in the document. */
class XclExpFontBuffer : public XclExpRecordBase, protected XclExpRoot
{
private:
    typedef XclExpRecordList< XclExpFont > XclExpFontList;

    XclExpFontList              maFontList;     /// List of all FONT records.
    XclFontData                 maAppFont;      /// Application font (for column width).
    sal_uInt32                  mnXclMaxCount;  /// Maximum number of fonts.

public:
    explicit                    XclExpFontBuffer( const XclExpRoot& rRoot );

    /** Finds the maximum number of fonts for the current BIFF version. */
    void                        OnChangeBiff();

    /** Returns the specified font from font list. */
    inline const XclExpFont*    GetFont( sal_uInt32 nIndex ) const
                                    { return maFontList.GetObject( nIndex ); }

    /** Searches for the passed font and returns its Excel font index.
        @return  Excel font index or nDefault, if not found. */
    sal_uInt16                  GetIndex( const XclExpFont& rFont, sal_uInt16 nDefault = 0 );
    /** Inserts the font into the buffer if not present, otherwise deletes it.
        @return  The resulting Excel font index. */
    sal_uInt16                  Insert( XclExpFont*& rpFont );
    /** Inserts the font into the buffer if not present.
        @return  The resulting Excel font index. */
    sal_uInt16                  Insert( const Font& rFont );
    /** Inserts the font contained in rAttr into the buffer if not present.
        @return  The resulting Excel font index. */
    sal_uInt16                  Insert( const ScPatternAttr& rAttr );

    /** Writes all FONT records contained in this buffer. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Initializes the default fonts for the current BIFF version. */
    void                        InitDefaultFonts();
    /** Converts a list index into Excel font index. */
    sal_uInt16                  GetXclIndex( sal_uInt32 nIndex ) const;
    /** Tries to find the passed font and returns the current list index. */
    sal_uInt32                  Find( const XclExpFont& rFont );
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
private:
    typedef ::std::auto_ptr< SvNumberFormatter >    SvNumberFormatterPtr;
    typedef NfKeywordTable*                         NfKeywordTablePtr;
    typedef ::std::vector< XclExpNumFmt >           XclExpNumFmtVec;

    SvNumberFormatterPtr        mpFormatter;    /// Special number formatter for conversion.
    NfKeywordTablePtr           mpKeywordTable; /// Replacement table.
    XclExpNumFmtVec             maFormatMap;    /// Maps core formats to Excel indexes.
    sal_uInt32                  mnStdFmt;       /// Key for standard number format.
    sal_uInt16                  mnXclOffset;    /// Offset to first user defined format.

public:
    explicit                    XclExpNumFmtBuffer( const XclExpRoot& rRoot );

    virtual                     ~XclExpNumFmtBuffer();

    /** Sets internal data for the current BIFF version. */
    void                        OnChangeBiff();

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
    /** Writes default formats occuring in each Excel file. */
    void                        WriteDefaultFormats( XclExpStream& rStrm );
};


// XF, STYLE record - Cell formatting =========================================

/** Contains color and line style for each cell border line. */
struct XclExpXFBorder
{
    sal_uInt32                  mnLeftColorId;      /// Color ID for left line.
    sal_uInt32                  mnRightColorId;     /// Color ID for right line.
    sal_uInt32                  mnTopColorId;       /// Color ID for top line.
    sal_uInt32                  mnBottomColorId;    /// Color ID for bottom line.
    sal_uInt8                   mnLeftLine;         /// Style of left line.
    sal_uInt8                   mnRightLine;        /// Style of right line.
    sal_uInt8                   mnTopLine;          /// Style of top line.
    sal_uInt8                   mnBottomLine;       /// Style of bottom line.

    explicit                    XclExpXFBorder();
};


// ----------------------------------------------------------------------------

/** Contains background colors and pattern. */
struct XclExpXFArea
{
    sal_uInt32                  mnForeColorId;  /// Foreground color ID.
    sal_uInt32                  mnBackColorId;  /// Background color ID.
    sal_uInt8                   mnPattern;      /// Fill pattern.

    explicit                    XclExpXFArea();
};


// ----------------------------------------------------------------------------

class SfxStyleSheetBase;
class SvxBoxItem;
class SvxBorderLine;

/** Represents an XF record which contains all formatting data of a cell or cell style. */
class XclExpXF : public XclExpRecord
{
private:
    typedef ::std::auto_ptr< ScPatternAttr > ScPatternAttrPtr;

    ScPatternAttrPtr            mpOwnPattern;       /// An own item set (used for cell styles).
    const ScPatternAttr*        mpPattern;          /// Pointer to Calc item set (we do not own it).

    XclExpXFBorder              maBorder;           /// Border line style.
    XclExpXFArea                maArea;             /// Background area style.
    XclHorAlign                 meHorAlign;         /// Horizontal alignment.
    XclVerAlign                 meVerAlign;         /// Vertical alignment.
    XclTextOrient               meOrient;           /// Text orientation.
    XclTextDirection            meTextDir;          /// CTL text direction.

    sal_uInt16                  mnParent;           /// Index to parent style XF.
    sal_uInt16                  mnFont;             /// Excel font index.
    sal_uInt16                  mnNumFmt;           /// Excel number format index.
    sal_uInt8                   mnIndent;           /// Text indent.
    sal_uInt8                   mnRotation;         /// Rotation angle.

    bool                        mbCellXF;           /// true = cell XF, false = style XF.
    bool                        mbLocked;           /// true = cell is locked.
    bool                        mbHidden;           /// true = formulas are hidden.
    bool                        mbWrapped;          /// true = wrap text on cell border.
    bool                        mbProtUsed;         /// true = locked/hidden flags used.
    bool                        mbFontUsed;         /// true = font index used.
    bool                        mbFmtUsed;          /// true = number format used.
    bool                        mbAlignUsed;        /// true = alignment used.
    bool                        mbBorderUsed;       /// true = border data used.
    bool                        mbAreaUsed;         /// true = area data used.

public:
    /** Constructs a cell XF record from the passed Calc item set.
        @param nForcedNumFmt  If not set to NUMBERFORMAT_ENTRY_NOT_FOUND, it will overwrite
        the number format of the passed item set.
        @param bForcedWrap  true = set text wrap flag unconditionally. */
    explicit                    XclExpXF(
                                    const XclExpRoot& rRoot,
                                    const ScPatternAttr* pPattern,
                                    sal_uInt32 nForcedNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND,
                                    bool bForceWrapped = false );
    /** Constructs a style XF record from the passed cell style sheet. */
    explicit                    XclExpXF( const XclExpRoot& rRoot, SfxStyleSheetBase& rStyleSheet );

    /** Returns true, if the passed cell formatting is represented by this XF record.
        @descr  Searches for cell XFs only. */
    bool                        Equals(
                                    const ScPatternAttr* pPattern,
                                    sal_uInt32 nForcedNumFmt,
                                    bool bForceWrapped ) const;

    /** Sets the border line styles from the item set into the passed struct. */
    static void                 GetBorder(
                                    XclExpXFBorder& rBorder,
                                    XclExpPalette& rPalette,
                                    const ScPatternAttr& rPattern );

    /** Sets the area styles from the brush item into the passed struct. */
    static void                 GetArea(
                                    XclExpXFArea& rArea,
                                    XclExpPalette& rPalette,
                                    const ScPatternAttr& rPattern );

private:
    /** Fills all members from the passed item set. */
    void                        Init(
                                    const XclExpRoot& rRoot,
                                    const ScPatternAttr* pPattern,
                                    sal_uInt32 nForcedNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND,
                                    bool bForceWrapped = false );

    /** Returns the bits specifying the used attributes.
        @descr  In cell XFs a set bit means a used attribute, in style XF a cleared
        bit means a used attribute. This method regards the cell/style state.
        @return  The mask based on bit 0 (not yet bit-shifted as needed for export). */
    sal_uInt8                   GetUsedAttribMask() const;

    void                        WriteBody5( XclExpStream& rStrm );
    void                        WriteBody8( XclExpStream& rStrm );

    /** Writes the contents of the XF record. */
    virtual void                WriteBody( XclExpStream& rStrm );

    /** Reads the passed border line struct and converts to Excel line style and color. */
    static void                 GetBorderLine(
                                    sal_uInt8& rnXclLine, sal_uInt32& rnColorId,
                                    XclExpPalette& rPalette, const SvxBorderLine* pLine );
};


// ----------------------------------------------------------------------------

/** Represents a STYLE record containing the name of a user-defined cell style. */
class XclExpStyle : public XclExpRecord
{
private:
    String                      maName;         /// Name of the cell style.
    sal_uInt16                  mnXFIndex;      /// Index to XF record with style formatting.

public:
    explicit                    XclExpStyle( const String& rName, sal_uInt16 nXFIndex );

private:
    /** Writes the contents of the STYLE record. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

/** Stores all XF records (cell formats) in the document. */
class XclExpXFBuffer : public XclExpRecordBase, protected XclExpRoot
{
private:
    typedef XclExpRecordList< XclExpXF >    XclExpXFList;
    typedef XclExpRecordList< XclExpStyle > XclExpStyleList;

    XclExpXFList                maXFList;       /// List of all XF records.
    XclExpStyleList             maStyleList;    /// List of all STYLE records.
    sal_uInt32                  mnXclMaxCount;  /// Maximum number of XF records.
    sal_uInt16                  mnXclOffset;    /// Offset to first user defined XF.

public:
    explicit                    XclExpXFBuffer( const XclExpRoot& rRoot );

    /** Inserts all user-defined styles into the XF record list. */
    void                        InsertUserStyles();

    /** Finds or creates a cell XF record for the passed item set.
        @param bForceWrapped  true = cell contains hard newlines.
        In this case the text wrap flag must be set in the XF record.
        @return  The resulting Excel XF record index. */
    sal_uInt16                  Insert( const ScPatternAttr* pPattern, bool bForceWrapped = false );
    /** Finds or creates a cell XF record for the passed item set, with custom number format.
        @param nForcedNumFmt  The number format to be exported, i.e. formula result type.
        This format will always overwrite the cell's number format.
        @return  The resulting Excel XF record index. */
    sal_uInt16                  Insert( const ScPatternAttr* pPattern, sal_uInt32 nForcedNumFmt );
    /** Inserts the passed cell style. Creates a style XF record and a STYLE record.
        @return  The resulting Excel XF record index. */
    sal_uInt16                  InsertStyle( SfxStyleSheetBase& rStyleSheet );

    /** Writes all XF records contained in this buffer. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Converts a list index into Excel XF index. */
    sal_uInt16                  GetXclIndex( sal_uInt32 nIndex ) const;

    /** Tries to find the XF record containing the passed format or inserts a new record.
        @return  The Excel XF index. */
    sal_uInt16                  InsertCellXF(
                                    const ScPatternAttr* pPattern,
                                    sal_uInt32 nForcedNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND,
                                    bool bForceWrapped = false );

    void                        WriteDefaultXFs5( XclExpStream& rStrm );
    void                        WriteDefaultXFs8( XclExpStream& rStrm );

    void                        WriteDefaultStyles5( XclExpStream& rStrm );
};


// ============================================================================

#endif

