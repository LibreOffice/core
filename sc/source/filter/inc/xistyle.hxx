/*************************************************************************
 *
 *  $RCSfile: xistyle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:13 $
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

#ifndef SC_XISTYLE_HXX
#define SC_XISTYLE_HXX

#ifndef _SVMEMPOOL_HXX
#include <tools/mempool.hxx>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif
#ifndef SC_SCPATATR_HXX
#include "patattr.hxx"
#endif

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

class XclImpStream;


/* ============================================================================
- Buffers for style records (PALETTE, FONT, FORMAT, XF)
and a container for XF indexes for every used cell in a sheet.
- Page styles import.
============================================================================ */

// PALETTE record - color information =========================================

/** Stores the default colors for the current BIFF version and the contents of
    a PALETTE record. */
class XclImpPalette : public XclDefaultPalette, protected XclImpRoot
{
private:
    typedef ::std::vector< ColorData > ColorDataVec;

    ColorDataVec                maColorTable;       /// Colors read from file.

public:
    explicit                    XclImpPalette( const XclImpRoot& rRoot );

    /** Activates the default colors for the current BIFF version. */
    void                        OnChangeBiff();

    /** Returns the RGB color data for a (non-zero-based) Excel palette entry.
        @descr  First looks for a color read from file, then looks for a default color.
        @param nDefault  Is returned, if nothing else could be found. */
    ColorData                   GetColorData( sal_uInt16 nXclIndex, ColorData nDefault = COL_AUTO ) const;
    /** Returns the color for a (non-zero-based) Excel palette entry.
        @descr  First looks for a color read from file, then looks for a default color.
        @param nDefault  Is returned, if nothing else could be found. */
    inline Color                GetColor( sal_uInt16 nXclIndex, ColorData nDefault = COL_AUTO ) const;

    /** Reads a PALETTE record. */
    void                        ReadPalette( XclImpStream& rStrm );
};

inline Color XclImpPalette::GetColor( sal_uInt16 nXclIndex, ColorData nDefault ) const
{
    return Color( GetColorData( nXclIndex, nDefault ) );
}


// FONT record - font information =============================================

/** Enumeration to choose the which IDs for font items. */
enum XclFontWhichIDMode
{
    xlFontScIDs,                /// Calc which IDs (ATTR_*).
    xlFontEEIDs,                /// Edit engine which IDs (EE_CHAR_*).
    xlFontHFIDs                 /// Header/footer edit engine which IDs (EE_CHAR_*).
};


// ----------------------------------------------------------------------------

/** Stores all data of an Excel font and provides import of FONT records. */
class XclImpFont : protected XclImpRoot
{
private:
    XclFontData                 maData;         /// All font attributes.
    FontFamily                  meFontFamily;   /// VCL font family for later use.
    CharSet                     meFontCharSet;  /// VCL font character set for later use.
    bool                        mbIsWestern;    /// true = font contains ASCII characters.
    bool                        mbIsAsian;      /// true = font contains CJK characters.
    bool                        mbIsComplex;    /// true = font contains CTL characters.

public:
    explicit                    XclImpFont( const XclImpRoot& rRoot );

    /** Constructs a font from font data.
        @descr  Special handling for font style (bold, italic) in font name,
        overwrites settings in rFontData. */
    explicit                    XclImpFont( const XclImpRoot& rRoot, const XclFontData& rFontData );

    /** Returns read-only access to font data. */
    inline const XclFontData&   GetFontData() const { return maData; }
    /** Returns true, if the font contains superscript or subscript. */
    inline bool                 HasEscapement() const { return maData.meEscapem != xlEscNone; }
    /** Returns the width of the '0' character on current printer (twips). */
    long                        GetCharWidth() const;

    /** Reads a FONT record for all BIFF versions. */
    void                        ReadFont( XclImpStream& rStrm );

    /** Fills all font properties to the item set.
        @param rItemSet  The destination item set.
        @param eMode  The type of which IDs. */
    void                        FillToItemSet(
                                    SfxItemSet& rItemSet,
                                    XclFontWhichIDMode eMode ) const;

    /** Calculates the Calc font family from the Excel family. */
    static FontFamily           GetScFontFamily( sal_uInt8 nXclFamily, const String& rName, CharSet eDefCharSet );
    /** Calculates the Calc font character set from the Excel character set. */
    static CharSet              GetScFontCharSet( sal_uInt8 nXclCharSet );
    /** Calculates the Calc font posture from the Excel posture. */
    static FontItalic           GetScFontPosture( bool bXclItalic );
    /** Calculates the Calc font weight from the Excel weight. */
    static FontWeight           GetScFontWeight( sal_uInt16 nXclWeight );
    /** Calculates the Calc font underline style from the Excel underline style. */
    static FontUnderline        GetScFontUnderline( XclUnderline eXclUnderl );
    /** Calculates the Calc escapement style from the Excel escapement style. */
    static SvxEscapement        GetScFontEscapement( XclEscapement eXclEscapem );

private:
    /** Reads and sets height and flags. */
    void                        ReadFontData2( XclImpStream& rStrm );
    /** Reads and sets height, flags, color, boldness, script, family and charset. */
    void                        ReadFontData5( XclImpStream& rStrm );

    /** Reads and sets a byte string as font name. */
    void                        ReadFontName2( XclImpStream& rStrm );
    /** Reads and sets a Unicode string as font name. */
    void                        ReadFontName8( XclImpStream& rStrm );

    /** Tests whether the font contains CJK or CTL characters.
        @descr  This is only a weak guess using preselected characters. */
    void                        GuessScriptType();
    /** Updates char set, font family and script types after reading or setting data. */
    void                        Update();
};


// ----------------------------------------------------------------------------

/** Stores the data of all fonts occurred in an Excel file. */
class XclImpFontBuffer : protected XclImpRoot, ScfNoCopy
{
private:
    ScfDelList< XclImpFont >    maFontList;     /// List of all FONT records in the Excel file.
    XclFontData                 maAppFont;      /// Application font (for column width).

public:
    explicit                    XclImpFontBuffer( const XclImpRoot& rRoot );

    /** Returns the object that stores all contents of a FONT record. */
    inline const XclImpFont*    GetFont( sal_uInt16 nFontIndex ) const
                                    { return maFontList.GetObject( nFontIndex ); }
    /** Returns the application font data of this file, needed i.e. for column width. */
    inline const XclFontData&   GetAppFontData() const { return maAppFont; }

    /** Reads a FONT record. */
    void                        ReadFont( XclImpStream& rStrm );

    /** Fills all font properties to the item set.
        @param nFontIndex  The Excel index of the font.
        @param rItemSet  The destination item set.
        @param eMode  The type of which IDs. */
    void                        FillToItemSet(
                                    sal_uInt16 nFontIndex,
                                    SfxItemSet& rItemSet,
                                    XclFontWhichIDMode eMode ) const;
};


// FORMAT record - number formats =============================================

/** Stores all user defined number formats occured in the file. */
class XclImpNumFmtBuffer : protected XclImpRoot, ScfNoCopy
{
private:
    ScfUInt32Vec                maKeyVec;       /// Array of SvNumberFomatter format keys.
    sal_uInt32                  mnStdFmt;       /// Key for standard number format.

public:
    explicit                    XclImpNumFmtBuffer( const XclImpRoot& rRoot );

    /** Returns the format key with the Excel index nIndex or standard key, if invalid index. */
    sal_uInt32                  GetFormat( sal_uInt32 nIndex );

    /** Reads a FORMAT record. */
    void                        ReadFormat( XclImpStream& rStrm );

private:
    /** Inserts the built-in number formats that Excel omits in BIFF5+. */
    void                        InsertBuiltinFormats();

    /** Inserts a format key exactly at the given position.
        @descr  The list will be extended, if it is too short, using the standard format key. */
    void                        InsertKey( sal_uInt32 nIndex, sal_uInt32 nFormatKey );
};


// XF, STYLE record - Cell formatting =========================================

/** Contains color and line style for each cell border line. */
struct XclImpXFBorder
{
    sal_uInt16                  mnLeftColor;    /// Index to color for left line.
    sal_uInt16                  mnRightColor;   /// Index to color for right line.
    sal_uInt16                  mnTopColor;     /// Index to color for top line.
    sal_uInt16                  mnBottomColor;  /// Index to color for bottom line.
    sal_uInt8                   mnLeftLine;     /// Style of left line.
    sal_uInt8                   mnRightLine;    /// Style of right line.
    sal_uInt8                   mnTopLine;      /// Style of top line.
    sal_uInt8                   mnBottomLine;   /// Style of bottom line.
};


// ----------------------------------------------------------------------------

/** Contains background colors and pattern. */
struct XclImpXFArea
{
    sal_uInt16                  mnForeColor;    /// Index to foreground color.
    sal_uInt16                  mnBackColor;    /// Index to background color.
    sal_uInt8                   mnPattern;      /// Fill pattern.
};


// ----------------------------------------------------------------------------

class SvxBorderLine;

/** Contains all data of a XF record and a Calc item set. */
class XclImpXF : protected XclImpRoot, ScfNoCopy
{
private:
    typedef ::std::auto_ptr< ScPatternAttr >    ScPatternAttrPtr;

    mutable ScPatternAttrPtr    mpPattern;          /// Calc item set.

    XclImpXFBorder              maBorder;           /// Border line style.
    XclImpXFArea                maArea;             /// Background area style.
    XclHorAlign                 meHorAlign;         /// Horizontal alignment.
    XclVerAlign                 meVerAlign;         /// Vertical alignment.
    XclTextOrient               meOrient;           /// Text orientation.
    XclTextDirection            meTextDir;          /// CTL text direction.

    sal_uInt16                  mnParent;           /// Index to parent style XF.
    sal_uInt32                  mnNumFmt;           /// Id for value format.
    sal_uInt16                  mnFont;             /// Index to font record.
    sal_uInt16                  mnIndent;           /// Text indent.
    sal_uInt8                   mnRotation;         /// Rotation angle.

    bool                        mbCellXF;           /// true = cell XF, false = style XF.
    bool                        mbLocked;           /// true = cell is locked.
    bool                        mbHidden;           /// true = formulas are hidden.
    bool                        mbWrapped;          /// true = automatic line break.
    bool                        mbProtUsed;         /// true = locked/hidden flags used.
    bool                        mbFontUsed;         /// true = font index used.
    bool                        mbFmtUsed;          /// true = number format used.
    bool                        mbAlignUsed;        /// true = alignment used.
    bool                        mbBorderUsed;       /// true = border data used.
    bool                        mbAreaUsed;         /// true = area data used.

public:
    explicit                    XclImpXF( const XclImpRoot& rRoot );
                                ~XclImpXF();

    /** Reads an XF record. */
    void                        ReadXF( XclImpStream& rStrm );

    /** Creates a Calc item set containing an item set with all cell properties.
        @return  A read-only reference to the item set stored internally. */
    const ScPatternAttr&        GetPattern() const;

    inline bool                 IsCellXF() const    { return mbCellXF; }
    inline bool                 IsStyleXF() const   { return !IsCellXF(); }

    inline XclHorAlign          GetHorAlign() const { return meHorAlign; }
    inline XclVerAlign          GetVerAlign() const { return meVerAlign; }
    inline sal_uInt16           GetFont() const     { return mnFont; }

    /** Inserts the border line styles into the item set. */
    static void                 SetBorder(
                                    SfxItemSet& rItemSet,
                                    const XclImpPalette& rPalette,
                                    const XclImpXFBorder& rBorder );

    /** Inserts the area style into the item set. */
    static void                 SetArea(
                                    SfxItemSet& rItemSet,
                                    const XclImpPalette& rPalette,
                                    const XclImpXFArea& rArea );

private:
    void                        ReadXF2( XclImpStream& rStrm );
    void                        ReadXF3( XclImpStream& rStrm );
    void                        ReadXF4( XclImpStream& rStrm );
    void                        ReadXF5( XclImpStream& rStrm );
    void                        ReadXF8( XclImpStream& rStrm );

    /** Creates a new border line struct from passed line style. */
    static SvxBorderLine*       CreateBorderLine(
                                    const XclImpPalette& rPalette,
                                    sal_uInt8 nXclLine, sal_uInt16 nXclColor );
};


// ----------------------------------------------------------------------------

/** Contains all XF records occured in the file.
    @descr  This class is able to read XF records (BIFF2 - BIFF8) and STYLE records (BIFF8). */
class XclImpXFBuffer : protected XclImpRoot, ScfNoCopy
{
private:
    ScfDelList< XclImpXF >      maXFList;       /// List of contents of all XF record.
    ScPatternAttr               maDefPattern;   /// Used if no styles could be found or created.

public:
    explicit                    XclImpXFBuffer( const XclImpRoot& rRoot );

    /** Reads an XF record. */
    void                        ReadXF( XclImpStream& rStrm );
    /** Reads a STYLE record. */
    void                        ReadStyle( XclImpStream& rStrm );

    /** Creates a item set with all properties stored in the XF record.
        @return  A read-only reference to the item set stored internally. */
    const ScPatternAttr&        GetPattern( sal_uInt32 nXFIndex ) const;

    /** Returns the object that stores all contents of an XF record. */
    inline const XclImpXF*      GetXF( sal_uInt32 nXFIndex ) const
                                    { return maXFList.GetObject( nXFIndex ); }
    /** Returns the index to the Excel font used in this XF record. */
    sal_uInt16                  GetFontIndex( sal_uInt32 nXFIndex ) const;
    /** Returns true, if either superscript or subscript is used in the font. */
    bool                        HasEscapement( sal_uInt32 nXFIndex ) const;
};


// Buffer for XF indexes in cells =============================================

/** Contains an XF index for a range of rows in a single column. */
class XclImpXFIndex
{
    DECL_FIXEDMEMPOOL_NEWDEL( XclImpXFIndex )

public:
    sal_uInt16                  mnFirstRow;     /// The first row of an equal-formatted range.
    sal_uInt16                  mnLastRow;      /// The last row of an equal-formatted range.
    sal_uInt16                  mnXF;           /// Index to the XF record.

    inline explicit             XclImpXFIndex( sal_uInt16 nRow, sal_uInt16 nXFIndex );
    inline explicit             XclImpXFIndex( sal_uInt16 nFRow, sal_uInt16 nLRow, sal_uInt16 nXFIndex );

    /** Returns true, if nRow is contained in own row range. */
    inline bool                 Contains( sal_uInt16 nRow ) const;

    /** Returns true, if the range has been expanded. */
    bool                        Expand( sal_uInt16 nRow, sal_uInt16 nXFIndex );
    /** Returns true, if the range has been expanded. */
    bool                        Expand( const XclImpXFIndex& rNextStyle );
};

inline XclImpXFIndex::XclImpXFIndex( sal_uInt16 nRow, sal_uInt16 nXFIndex ) :
    mnFirstRow( nRow ),
    mnLastRow( nRow ),
    mnXF( nXFIndex )
{
}

inline XclImpXFIndex::XclImpXFIndex( sal_uInt16 nFRow, sal_uInt16 nLRow, sal_uInt16 nXFIndex ) :
    mnFirstRow( nFRow ),
    mnLastRow( nLRow ),
    mnXF( nXFIndex )
{
}

inline bool XclImpXFIndex::Contains( sal_uInt16 nRow ) const
{
    return (mnFirstRow <= nRow) && (nRow <= mnLastRow);
}


// ----------------------------------------------------------------------------

/** Contains the XF indexes for every used cell in a column. */
class XclImpXFIndexColumn : ScfNoCopy
{
private:
    ScfDelList< XclImpXFIndex > maIndexList;    /// The list of cell range styles.

public:
    inline explicit             XclImpXFIndexColumn() {}

    /** Returns the first formatted cell range in this column. */
    inline XclImpXFIndex*       First()     { return maIndexList.First(); }
    /** Returns the next formatted cell range in this column. */
    inline XclImpXFIndex*       Next()      { return maIndexList.Next(); }

    /** Inserts a new XF index (first try to expand the last range). */
    void                        SetXF( sal_uInt16 nRow, sal_uInt16 nXFIndex );

private:
    /** Finds the previous and next row range from row position nRow.
        @descr  If an XF still exists, it is contained in rpPrevStyle. */
    void                        Find(
                                    XclImpXFIndex*& rpPrevStyle,
                                    XclImpXFIndex*& rpNextStyle,
                                    sal_uInt32& rnNextIndex,
                                    sal_uInt16 nRow ) const;

    /** Tries to concatenate a range with its predecessor.
        @descr  The ranges must have the same XF index and must not have a gap.
        The resulting range has the index nIndex-1. */
    void                        TryConcatPrev( sal_uInt32 nIndex );
};


// ----------------------------------------------------------------------------

/** Contains the XF indexes for every used cell in a single sheet. */
class XclImpXFIndexBuffer : protected XclImpRoot, ScfNoCopy
{
private:
    /** Insertion mode of an XF index. */
    enum XclImpXFInsertMode
    {
        xlXFModeCell,               /// Filled cell.
        xlXFModeBlank,              /// Blank cell.
        xlXFModeRow                 /// Row default XF.
    };

    typedef ::std::auto_ptr< XclImpXFIndexColumn > XclImpXFIndexColumnPtr;

    XclImpXFIndexColumnPtr*     mppColumns;     /// Array of pointers to column XF index buffers.
    sal_uInt32                  mnUsedCount;    /// Column width of formatted columns (last used + 1).

    ScRangeList                 maMergeList;    /// List of merged cell ranges.

public:
    explicit                    XclImpXFIndexBuffer( const XclImpRoot& rRoot );
    virtual                     ~XclImpXFIndexBuffer();

    /** Inserts a new XF index. */
    void                        SetXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex );
    /** Inserts a new XF index for blank cells. */
    void                        SetBlankXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex );
    /** Inserts a new XF index for all cells in a row. */
    void                        SetRowDefXF( sal_uInt16 nRow, sal_uInt16 nXFIndex );

    /** Inserts the first cell of a merged cell range. */
    void                        SetMerge( sal_uInt16 nCol, sal_uInt16 nRow );
    /** Inserts a complete merged cell range. */
    void                        SetMerge(
                                    sal_uInt16 nFirstCol, sal_uInt16 nFirstRow,
                                    sal_uInt16 nLastCol, sal_uInt16 nLastRow );

    /** Applies styles and cell merging to the current sheet in the document. */
    void                        Apply();

private:
    /** Clears all buffered data, used to set up for a new sheet. */
    void                        Clear();

    /** Inserts a new XF index for the specified cell type. */
    void                        SetXF(
                                    sal_uInt16 nCol, sal_uInt16 nRow,
                                    sal_uInt16 nXFIndex, XclImpXFInsertMode eMode );

    /** Copies border of the last cell of the range to the first cell to keep it visible
        when the range is merged.
        @param  nLine
        BOX_LINE_RIGHT = copy most-right border of top row;
        BOX_LINE_BOTTOM = copy most-bottom border of first column. */
    void                        SetBorderLine( const ScRange& rRange, sal_uInt16 nTab, sal_uInt16 nLine );
};


// ============================================================================

#endif

