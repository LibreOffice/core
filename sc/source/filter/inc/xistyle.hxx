/*************************************************************************
 *
 *  $RCSfile: xistyle.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-16 08:20:03 $
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
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
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
class ScDocumentPool;


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
public:
    explicit                    XclImpPalette( const XclImpRoot& rRoot );

    /** Activates the default colors for the current BIFF version. */
    void                        OnChangeBiff();

    /** Returns the RGB color data for a (non-zero-based) Excel palette entry.
        @descr  First looks for a color read from file, then looks for a default color.
        @return  The color from current or default palette or COL_AUTO, if nothing else found. */
    ColorData                   GetColorData( sal_uInt16 nXclIndex ) const;
    /** Returns the color for a (non-zero-based) Excel palette entry.
        @descr  First looks for a color read from file, then looks for a default color.
        @return  The color from current or default palette or COL_AUTO, if nothing else found. */
    inline Color                GetColor( sal_uInt16 nXclIndex ) const
                                    { return Color( GetColorData( nXclIndex ) ); }

    /** Reads a PALETTE record. */
    void                        ReadPalette( XclImpStream& rStrm );

private:
    typedef ::std::vector< ColorData > ColorDataVec;
    ColorDataVec                maColorTable;       /// Colors read from file.
};


// FONT record - font information =============================================

/** Enumeration to choose the Which-IDs for font items. */
enum XclFontWhichIDMode
{
    xlFontScIDs,                /// Calc Which-IDs (ATTR_*).
    xlFontEEIDs,                /// Edit engine Which-IDs (EE_CHAR_*).
    xlFontHFIDs                 /// Header/footer edit engine Which-IDs (EE_CHAR_*).
};


// ----------------------------------------------------------------------------

/** Stores all data of an Excel font and provides import of FONT records. */
class XclImpFont : protected XclImpRoot
{
public:
    explicit                    XclImpFont( const XclImpRoot& rRoot );

    /** Constructs a font from font data.
        @descr  Special handling for font style (bold, italic) in font name,
        overwrites settings in rFontData. */
    explicit                    XclImpFont( const XclImpRoot& rRoot, const XclFontData& rFontData );

    /** Sets all font attributes to used or unused. */
    void                        SetAllUsedFlags( bool bUsed );

    /** Returns read-only access to font data. */
    inline const XclFontData&   GetFontData() const { return maData; }
    /** Returns true, if the font contains superscript or subscript. */
    inline bool                 HasEscapement() const { return maData.mnEscapem != EXC_FONTESC_NONE; }

    /** Returns true, if this font contains ASCII characters. */
    inline bool                 HasAscii() const { return mbAscii; }
    /** Returns true, if this font contains CJK characters. */
    inline bool                 HasCjk() const { return mbCjk; }
    /** Returns true, if this font contains CTL characters. */
    inline bool                 HasCtl() const { return mbCtl; }

    /** Reads a FONT record for all BIFF versions. */
    void                        ReadFont( XclImpStream& rStrm );
    /** Reads the font block from a CF (conditional format) record. */
    void                        ReadCFFontBlock( XclImpStream& rStrm );

    /** Fills all font properties to the item set.
        @param rItemSet  The destination item set.
        @param eMode  The type of Which-IDs.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                        FillToItemSet(
                                    SfxItemSet& rItemSet,
                                    XclFontWhichIDMode eMode,
                                    bool bSkipPoolDefs = false ) const;

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

private:
    XclFontData                 maData;         /// All font attributes.
    bool                        mbAscii;        /// true = font contains ASCII characters.
    bool                        mbCjk;          /// true = font contains CJK characters.
    bool                        mbCtl;          /// true = font contains CTL characters.
    bool                        mbFontNameUsed; /// true = Font name, family, charset used.
    bool                        mbHeightUsed;   /// true = Font height used.
    bool                        mbColorUsed;    /// true = Color used.
    bool                        mbWeightUsed;   /// true = Weight used.
    bool                        mbEscapemUsed;  /// true = Escapement type used.
    bool                        mbUnderlUsed;   /// true = Underline style used.
    bool                        mbItalicUsed;   /// true = Italic used.
    bool                        mbStrikeUsed;   /// true = Strikeout used.
    bool                        mbOutlineUsed;  /// true = Outlined used.
    bool                        mbShadowUsed;   /// true = Shadowed used.
};


// ----------------------------------------------------------------------------

/** Stores the data of all fonts occurred in an Excel file. */
class XclImpFontBuffer : protected XclImpRoot, ScfNoCopy
{
public:
    explicit                    XclImpFontBuffer( const XclImpRoot& rRoot );

    /** Returns the object that stores all contents of a FONT record. */
    inline const XclImpFont*    GetFont( sal_uInt16 nFontIndex ) const
                                    { return maFontList.GetObject( nFontIndex ); }
    /** Returns the application font data of this file, needed i.e. for column width. */
    inline const XclFontData&   GetAppFontData() const { return maAppFont; }

    /** Reads a FONT record. */
    void                        ReadFont( XclImpStream& rStrm );

    /** Fills all font properties from a FONT record to the item set.
        @param rItemSet  The destination item set.
        @param eMode  The type of Which-IDs.
        @param nFontIndex  The Excel index of the font.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                        FillToItemSet(
                                    SfxItemSet& rItemSet,
                                    XclFontWhichIDMode eMode,
                                    sal_uInt16 nFontIndex,
                                    bool bSkipPoolDefs = false ) const;

private:
    ScfDelList< XclImpFont >    maFontList;     /// List of all FONT records in the Excel file.
    XclFontData                 maAppFont;      /// Application font (for column width).
};


// FORMAT record - number formats =============================================

/** Stores all user defined number formats occured in the file. */
class XclImpNumFmtBuffer : protected XclImpRoot, ScfNoCopy
{
public:
    explicit                    XclImpNumFmtBuffer( const XclImpRoot& rRoot );

    /** Returns the format key with the Excel index nNumFmt or standard key, if invalid index. */
    sal_uInt32                  GetFormat( sal_uInt16 nNumFmt ) const;
    /** Returns the core index of the current standard number format. */
    inline sal_uInt32           GetStandardFormat() const { return mnStdFmt; }

    /** Reads a FORMAT record. */
    void                        ReadFormat( XclImpStream& rStrm );

    /** Fills an Excel number format to the passed item set.
        @param rItemSet  The destination item set.
        @param nNumFmt  The Excel number format index.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                        FillToItemSet(
                                    SfxItemSet& rItemSet, sal_uInt16 nNumFmt,
                                    bool bSkipPoolDefs = false ) const;
    /** Fills a Calc number format to the passed item set.
        @param rItemSet  The destination item set.
        @param nScNumFmt  The Calc number formatter index of the format.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                        FillScFmtToItemSet(
                                    SfxItemSet& rItemSet, sal_uInt32 nScNumFmt,
                                    bool bSkipPoolDefs = false ) const;

private:
    /** Inserts the built-in number formats that Excel omits in BIFF5+. */
    void                        InsertBuiltinFormats();

    /** Inserts a format key exactly at the given position.
        @descr  The list will be extended, if it is too short, using the standard format key. */
    void                        InsertKey( sal_uInt32 nIndex, sal_uInt32 nFormatKey );

private:
    ScfUInt32Vec                maKeyVec;       /// Array of SvNumberFomatter format keys.
    sal_uInt32                  mnStdFmt;       /// Key for standard number format.
};


// XF, STYLE record - Cell formatting =========================================

/** Extends the XclCellProt struct for import.
    @descr  Provides functions to fill from Excel record data and to fill to item sets. */
struct XclImpCellProt : public XclCellProt
{
    /** Fills this struct with BIFF2 XF record data. */
    void                        FillFromXF2( sal_uInt8 nNumFmt );
    /** Fills this struct with BIFF3-BIFF8 XF record data. */
    void                        FillFromXF3( sal_uInt16 nProt );

    /** Inserts items representing this protection style into the item set.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                        FillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs = false ) const;
};


// ----------------------------------------------------------------------------

/** Extends the XclCellAlign struct for import.
    @descr  Provides functions to fill from Excel record data and to fill to item sets. */
struct XclImpCellAlign : public XclCellAlign
{
    /** Fills this struct with BIFF2 XF record data. */
    void                        FillFromXF2( sal_uInt8 nFlags );
    /** Fills this struct with BIFF3 XF record data. */
    void                        FillFromXF3( sal_uInt16 nAlign );
    /** Fills this struct with BIFF4 XF record data. */
    void                        FillFromXF4( sal_uInt16 nAlign );
    /** Fills this struct with BIFF5/BIFF7 XF record data. */
    void                        FillFromXF5( sal_uInt16 nAlign );
    /** Fills this struct with BIFF8 XF record data. */
    void                        FillFromXF8( sal_uInt16 nAlign, sal_uInt16 nMiscAttrib );

    /** Inserts items representing this alignment style into the item set.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                        FillToItemSet( SfxItemSet& rItemSet, const XclImpFont* pFont, bool bSkipPoolDefs = false ) const;
};


// ----------------------------------------------------------------------------

/** Extends the XclCellBorder struct for import.
    @descr  Provides functions to fill from Excel record data and to fill to item sets. */
struct XclImpCellBorder : public XclCellBorder
{
    bool                        mbLeftUsed;     /// true = Left line style used.
    bool                        mbRightUsed;    /// true = Right line style used.
    bool                        mbTopUsed;      /// true = Top line style used.
    bool                        mbBottomUsed;   /// true = Bottom line style used.

    explicit                    XclImpCellBorder();

    /** Sets all line states to used or unused. */
    void                        SetAllUsedFlags( bool bUsed );

    /** Fills this struct with BIFF2 XF record data. */
    void                        FillFromXF2( sal_uInt8 nFlags );
    /** Fills this struct with BIFF3/BIFF4 XF record data. */
    void                        FillFromXF3( sal_uInt32 nBorder );
    /** Fills this struct with BIFF5/BIFF7 XF record data. */
    void                        FillFromXF5( sal_uInt32 nBorder, sal_uInt32 nArea );
    /** Fills this struct with BIFF8 XF record data. */
    void                        FillFromXF8( sal_uInt32 nBorder1, sal_uInt32 nBorder2 );

    /** Fills this struct with BIFF8 CF (conditional format) record data. */
    void                        FillFromCF8( sal_uInt16 nLineStyle, sal_uInt32 nLineColor, sal_uInt32 nFlags );

    /** Inserts a box item representing this border style into the item set.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                        FillToItemSet(
                                    SfxItemSet& rItemSet,
                                    const XclImpPalette& rPalette,
                                    bool bSkipPoolDefs = false ) const;
};


// ----------------------------------------------------------------------------

/** Extends the XclCellArea struct for import.
    @descr  Provides functions to fill from Excel record data and to fill to item sets. */
struct XclImpCellArea : public XclCellArea
{
    bool                        mbForeUsed;     /// true = Foreground color used.
    bool                        mbBackUsed;     /// true = Background color used.
    bool                        mbPattUsed;     /// true = Pattern used.

    explicit                    XclImpCellArea();

    /** Sets colors and pattern state to used or unused. */
    void                        SetAllUsedFlags( bool bUsed );

    /** Fills this struct with BIFF2 XF record data. */
    void                        FillFromXF2( sal_uInt8 nFlags );
    /** Fills this struct with BIFF3/BIFF4 XF record data. */
    void                        FillFromXF3( sal_uInt16 nArea );
    /** Fills this struct with BIFF5/BIFF7 XF record data. */
    void                        FillFromXF5( sal_uInt32 nArea );
    /** Fills this struct with BIFF8 XF record data. */
    void                        FillFromXF8( sal_uInt32 nBorder2, sal_uInt16 nArea );

    /** Fills this struct with BIFF8 CF (conditional format) record data. */
    void                        FillFromCF8( sal_uInt16 nPattern, sal_uInt16 nColor, sal_uInt32 nFlags );

    /** Inserts a brush item representing this area style into the item set.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items. */
    void                        FillToItemSet(
                                    SfxItemSet& rItemSet,
                                    const XclImpPalette& rPalette,
                                    bool bSkipPoolDefs = false ) const;
};


// ----------------------------------------------------------------------------

/** Contains all data of a XF record and a Calc item set. */
class XclImpXF : public XclXFBase, protected XclImpRoot, ScfNoCopy
{
public:
    explicit                    XclImpXF( const XclImpRoot& rRoot );
                                ~XclImpXF();

    /** Reads an XF record. */
    void                        ReadXF( XclImpStream& rStrm );

    /** Sets the style name of this XF, if it is a style XF.
        @descr  Additionally creates this user-defined style in the Calc document. */
    void                        SetStyleName( const String& rStyleName );
    /** Sets the style name of this XF from a built-in Excel style, if it is a style XF.
        @descr  Does not create the style in the Calc document. This is done on demand
        in CreatePattern(), if the style is really used. */
    void                        SetBuiltInStyleName( sal_uInt8 nStyleId, sal_uInt8 nLevel );

    inline const String&        GetStyleName() const    { return maStyleName; }
    inline XclHorAlign          GetHorAlign() const     { return maAlignment.meHorAlign; }
    inline XclVerAlign          GetVerAlign() const     { return maAlignment.meVerAlign; }
    inline sal_uInt16           GetFont() const         { return mnFont; }

    /** Inserts all formatting attributes to the specified area in the Calc document.
        @param nForcedNumFmt  If not set to NUMBERFORMAT_ENTRY_NOT_FOUND, it will overwrite
        the number format of the XF. */
    void                        ApplyPattern(
                                    sal_uInt16 nFirstCol, sal_uInt16 nFirstRow,
                                    sal_uInt16 nLastCol, sal_uInt16 nLastRow,
                                    sal_uInt16 nTab,
                                    sal_uInt32 nForcedNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND );

private:
    void                        ReadXF2( XclImpStream& rStrm );
    void                        ReadXF3( XclImpStream& rStrm );
    void                        ReadXF4( XclImpStream& rStrm );
    void                        ReadXF5( XclImpStream& rStrm );
    void                        ReadXF8( XclImpStream& rStrm );

    /** Sets all "attribute used" flags according to the passed mask.
        @descr  In cell XFs, a set bit represents "used", in style XFs it is a cleared bit.
        Therefore mbCellXF must be set correctly before calling this method. */
    void                        SetUsedFlags( sal_uInt8 nUsedFlags );
    /** Sets own "attribute used" flags, if attributes are different from passed parent XF. */
    void                        UpdateUsedFlags( const XclImpXF& rParentXF );

    /** Creates a Calc item set containing an item set with all cell properties.
        @param bSkipPoolDefs  true = Do not put items equal to pool default; false = Put all items.
        @return  A read-only reference to the item set stored internally. */
    const ScPatternAttr&        CreatePattern( bool bSkipPoolDefs = false );
    /** Creates a cell style sheet and inserts it into the Calc document.
        @descr  Creates a style sheet only for style XFs with a valid style name.
        @return  The pointer to the cell style sheet, or 0, if there is no style sheet. */
    ScStyleSheet*               CreateStyleSheet();

private:
    typedef ::std::auto_ptr< ScPatternAttr >    ScPatternAttrPtr;

    ScPatternAttrPtr            mpPattern;          /// Calc item set.
    String                      maStyleName;        /// Name of the style sheet.
    ScStyleSheet*               mpStyleSheet;       /// Calc cell style sheet.

    XclImpCellProt              maProtection;       /// Cell protection flags.
    XclImpCellAlign             maAlignment;        /// All alignment attributes.
    XclImpCellBorder            maBorder;           /// Border line style.
    XclImpCellArea              maArea;             /// Background area style.
    sal_uInt16                  mnNumFmt;           /// Index to number format.
    sal_uInt16                  mnFont;             /// Index to font record.

    bool                        mbWasBuiltIn;       /// true = XF was an Excel built-in style.
};


// ----------------------------------------------------------------------------

/** Contains all XF records occured in the file.
    @descr  This class is able to read XF records (BIFF2 - BIFF8) and STYLE records (BIFF8). */
class XclImpXFBuffer : protected XclImpRoot, ScfNoCopy
{
public:
    explicit                    XclImpXFBuffer( const XclImpRoot& rRoot );

    /** Reads an XF record. */
    void                        ReadXF( XclImpStream& rStrm );
    /** Reads a STYLE record. */
    void                        ReadStyle( XclImpStream& rStrm );

    /** Returns the object that stores all contents of an XF record. */
    inline XclImpXF*            GetXF( sal_uInt32 nXFIndex ) const
                                    { return maXFList.GetObject( nXFIndex ); }
    /** Returns the index to the Excel font used in this XF record. */
    sal_uInt16                  GetFontIndex( sal_uInt32 nXFIndex ) const;
    /** Returns true, if either superscript or subscript is used in the font. */
    bool                        HasEscapement( sal_uInt32 nXFIndex ) const;

    /** Inserts formatting attributes from an XF to the specified area in the Calc document.
        @param nForcedNumFmt  If not set to NUMBERFORMAT_ENTRY_NOT_FOUND, it will overwrite
        the number format of the XF. */
    void                        ApplyPattern(
                                    sal_uInt16 nFirstCol, sal_uInt16 nFirstRow,
                                    sal_uInt16 nLastCol, sal_uInt16 nLastRow,
                                    sal_uInt16 nTab, sal_uInt16 nXFIndex,
                                    sal_uInt32 nForcedNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND );

private:
    ScfDelList< XclImpXF >      maXFList;       /// List of contents of all XF record.
};


// Buffer for XF indexes in cells =============================================

/** Contains an (encoded) XF index for a range of rows in a single column. */
class XclImpXFIndex
{
    DECL_FIXEDMEMPOOL_NEWDEL( XclImpXFIndex )

public:
    sal_uInt16                  mnFirstRow;     /// The first row of an equal-formatted range.
    sal_uInt16                  mnLastRow;      /// The last row of an equal-formatted range.
    sal_uInt32                  mnEncXF;        /// Encoded XF index (contains additional information).

    inline explicit             XclImpXFIndex( sal_uInt16 nRow, sal_uInt32 nEncXFIndex );
    inline explicit             XclImpXFIndex( sal_uInt16 nFRow, sal_uInt16 nLRow, sal_uInt32 nEncXFIndex );

    /** Returns true, if nRow is contained in own row range. */
    inline bool                 Contains( sal_uInt16 nRow ) const;

    /** Returns true, if the range has been expanded. */
    bool                        Expand( sal_uInt16 nRow, sal_uInt32 nEncXFIndex );
    /** Returns true, if the range has been expanded. */
    bool                        Expand( const XclImpXFIndex& rNextStyle );
};

inline XclImpXFIndex::XclImpXFIndex( sal_uInt16 nRow, sal_uInt32 nEncXFIndex ) :
    mnFirstRow( nRow ),
    mnLastRow( nRow ),
    mnEncXF( nEncXFIndex )
{
}

inline XclImpXFIndex::XclImpXFIndex( sal_uInt16 nFRow, sal_uInt16 nLRow, sal_uInt32 nEncXFIndex ) :
    mnFirstRow( nFRow ),
    mnLastRow( nLRow ),
    mnEncXF( nEncXFIndex )
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
public:
    inline explicit             XclImpXFIndexColumn() {}

    /** Returns the first formatted cell range in this column. */
    inline XclImpXFIndex*       First()     { return maIndexList.First(); }
    /** Returns the next formatted cell range in this column. */
    inline XclImpXFIndex*       Next()      { return maIndexList.Next(); }

    /** Inserts a new (encoded) XF index (first try to expand the last range). */
    void                        SetXF( sal_uInt16 nRow, sal_uInt32 nEncXFIndex );

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

private:
    ScfDelList< XclImpXFIndex > maIndexList;    /// The list of cell range styles.
};


// ----------------------------------------------------------------------------

/** Contains the XF indexes for every used cell in a single sheet. */
class XclImpXFIndexBuffer : protected XclImpRoot, ScfNoCopy
{
public:
    explicit                    XclImpXFIndexBuffer( const XclImpRoot& rRoot );
    virtual                     ~XclImpXFIndexBuffer();

    /** Inserts a new XF index. */
    void                        SetXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex );
    /** Inserts a new XF index for blank cells. */
    void                        SetBlankXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex );
    /** Inserts a new XF index for boolean cells. */
    void                        SetBoolXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex );
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
    /** Insertion mode of an XF index. */
    enum XclImpXFInsertMode
    {
        xlXFModeCell,               /// Filled cell.
        xlXFModeBoolCell,           /// Cell with a single Boolean value.
        xlXFModeBlank,              /// Blank cell.
        xlXFModeRow                 /// Row default XF.
    };

private:
    /** Clears all buffered data, used to set up for a new sheet. */
    void                        Clear();

    /** Encoded an XF index with additional information. */
    sal_uInt32                  EncodeXFIndex( sal_uInt16 nXFIndex, XclImpXFInsertMode eMode ) const;
    /** Encoded an XF index with additional information. */
    void                        DecodeXFIndex( sal_uInt16& rnXFIndex, sal_uInt32& rnForcedNumFmt, sal_uInt32 nEncXFIndex ) const;

    /** Inserts a new XF index for the specified cell type. */
    void                        SetXF(
                                    sal_uInt16 nCol, sal_uInt16 nRow,
                                    sal_uInt16 nXFIndex, XclImpXFInsertMode eMode );

    /** Copies border of the last cell of the range to the first cell to keep it visible
        when the range is merged.
        @param nLine
        BOX_LINE_RIGHT = copy most-right border of top row;
        BOX_LINE_BOTTOM = copy most-bottom border of first column. */
    void                        SetBorderLine( const ScRange& rRange, sal_uInt16 nTab, sal_uInt16 nLine );

private:
    typedef ::std::auto_ptr< XclImpXFIndexColumn > XclImpXFIndexColumnPtr;

    XclImpXFIndexColumnPtr*     mppColumns;     /// Array of pointers to column XF index buffers.
    sal_uInt32                  mnUsedCount;    /// Column width of formatted columns (last used + 1).

    ScRangeList                 maMergeList;    /// List of merged cell ranges.
};


// ============================================================================

#endif

