/*************************************************************************
 *
 *  $RCSfile: xestyle.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2002-11-21 12:11:13 $
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
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
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
    @descr  Supports color reduction to the maximum count of the current BIFF version. */
class XclExpPalette : public XclExpRecord, public XclDefaultPalette
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
        sal_uInt32              mnCID;      /// Unique color ID (CID) for color reduction.
        sal_uInt32              mnWeight;   /// Weighting for color reduction.

    public:
        explicit                XclListColor( const Color& rColor, sal_uInt32 nCID );

        /** Returns the RGB color value of the color. */
        inline const Color&     GetColor() const { return maColor; }
        /** Returns the unique ID of the color. */
        inline sal_uInt32       GetCID() const { return mnCID; }
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

    /** Data for each inserted original color, represented by a CID (color ID). */
    struct XclCIDData
    {
        Color                   maColor;        /// The original inserted color.
        sal_uInt32              mnIndex;        /// Maps current CID to color list or export color vector.
        /** Sets the contents of this struct. */
        inline void             Set( const Color& rColor, sal_uInt32 nIndex ) { maColor = rColor; mnIndex = nIndex; }
    };

    /** A color that will be written to the Excel file. */
    struct XclPaletteColor
    {
        Color                   maColor;        /// Resulting color to export.
        bool                    mbDefault;      /// true = Entry is a default color.
        /** Sets a new color.
            @param bDefault  true = Color is a color from default palette. */
        inline void             SetColor( const Color& rColor, bool bDefault = false ) { maColor = rColor; mbDefault = bDefault; }
    };

    typedef ScfDelList< XclListColor >          XclListColorList;
    typedef ::std::vector< XclCIDData >         XclCIDDataVec;
    typedef ::std::vector< XclPaletteColor >    XclPaletteColorVec;

    XclListColorList            maColorList;    /// Working color list.
    XclCIDDataVec               maCIDDataVec;   /// Data of all CIDs.
    XclPaletteColorVec          maPalette;      /// Contains resulting colors to export.
    sal_uInt32                  mnLastIx;       /// Last insertion index for search opt.

public:
    explicit                    XclExpPalette( const XclExpRoot& rRoot );

    /** Activates the default colors for the passed BIFF version. */
    void                        SetBiff( XclBiff eBiff );

    /** Inserts the color into the list and updates weighting.
        @param nAutoDefault  The Excel palette index for automatic color.
        @return  A unique ID for this color. */
    sal_uInt32                  InsertColor( const Color& rColor, XclColorType eType, sal_uInt16 nAutoDefault = 0 );
    /** Inserts the Excel color index into the list.
        @return  A unique ID, that will later return exactly the passed index. */
    sal_uInt32                  InsertIndex( sal_uInt16 nIndex );

    /** Reduces the color list to the maximum count of the current BIFF version. */
    void                        ReduceColors();

    /** Returns the Excel palette index of the color with passed color ID (CID). */
    sal_uInt16                  GetColorIndex( sal_uInt32 nCID ) const;
    /** Returns the Excel palette index of the passed color (searches for nearest color).
        @param nAutoDefault  The Excel palette index for automatic color. */
    sal_uInt16                  GetColorIndex( const Color& rColor, sal_uInt16 nAutoDefault = 0 ) const;

    /** Returns a foreground and background color for the two passed CIDs.
        @descr  If rnXclPattern contains a solid pattern, this function tries to find
        the two best fitting colors and a mix pattern (25%, 50% or 75%) for nCIDForegr.
        This will result in a better approximation to the passed foreground color. */
    void                        GetMixedColors(
                                    sal_uInt16& rnXclForeIx, sal_uInt16& rnXclBackIx, sal_uInt16& rnXclPattern,
                                    sal_uInt32 nCIDForegr, sal_uInt32 nCIDBackgr ) const;

    /** Returns the RGB color data for a (non-zero-based) Excel palette entry.
        @param nDefault  Is returned, if nothing else could be found. */
    ColorData                   GetColorData( sal_uInt16 nXclIndex, ColorData nDefault = COL_AUTO ) const;
    /** Returns the color for a (non-zero-based) Excel palette entry.
        @param nDefault  Is returned, if nothing else could be found. */
    inline Color                GetColor( sal_uInt16 nXclIndex, ColorData nDefault = COL_AUTO ) const;

private:
    /** Returns the original inserted color represented by nCID. */
    const Color&                GetOriginalColor( sal_uInt32 nCID ) const;

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
    sal_uInt32                  mnCID;          /// Unique color ID for text color.
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
    void                        SetCID( sal_uInt32 nCID );
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
    inline sal_uInt32           GetCID() const { return mnCID; }

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
    XclExpRecordList< XclExpFont > maFontList;  /// List of all FONT records.
    sal_uInt32                  mnMaxCount;     /// Maximum possible count of fonts.

public:
    explicit                    XclExpFontBuffer( const XclExpRoot& rRoot );

    /** Activates the default colors for the passed BIFF version. */
    void                        SetBiff( XclBiff eBiff );

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
    sal_uInt16                  GetXclIndex( sal_uInt32 nIndex );
    /** Tries to find the passed font and returns the current list index. */
    sal_uInt32                  Find( const XclExpFont& rFont );
};


// FORMAT record - number formats =============================================

// XF, STYLE record - Cell formatting =========================================

// ============================================================================

#endif

