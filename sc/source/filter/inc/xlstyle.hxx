/*************************************************************************
 *
 *  $RCSfile: xlstyle.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:43:32 $
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

// ============================================================================

#ifndef SC_XLSTYLE_HXX
#define SC_XLSTYLE_HXX

#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif


// Constants and Enumerations =================================================

// (0x001E, 0x041E) FORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_FORMAT              = 0x041E;

const sal_uInt16 EXC_FORMAT_OFFSET5         = 164;
const sal_uInt16 EXC_FORMAT_OFFSET8         = 164;


// (0x0031) FONT --------------------------------------------------------------

const sal_uInt16 EXC_ID_FONT                = 0x0031;

const sal_uInt16 EXC_FONT_APP               = 0;        /// Application font index.
const sal_uInt16 EXC_FONT_NOTFOUND          = 0xFFFF;

const sal_uInt32 EXC_FONT_MAXCOUNT4         = 0x00FF;
const sal_uInt32 EXC_FONT_MAXCOUNT5         = 0x00FF;
const sal_uInt32 EXC_FONT_MAXCOUNT8         = 0xFFFF;

// families
const sal_uInt8 EXC_FONTFAM_DONTKNOW        = 0x00;
const sal_uInt8 EXC_FONTFAM_ROMAN           = 0x01;
const sal_uInt8 EXC_FONTFAM_SWISS           = 0x02;
const sal_uInt8 EXC_FONTFAM_SYSTEM          = EXC_FONTFAM_SWISS;
const sal_uInt8 EXC_FONTFAM_MODERN          = 0x03;
const sal_uInt8 EXC_FONTFAM_SCRIPT          = 0x04;
const sal_uInt8 EXC_FONTFAM_DECORATIVE      = 0x05;

// charsets
const sal_uInt8 EXC_FONTCSET_ANSI_LATIN     = 0x00;

// attributes
const sal_uInt16 EXC_FONTATTR_NONE          = 0x0000;
const sal_uInt16 EXC_FONTATTR_BOLD          = 0x0001;
const sal_uInt16 EXC_FONTATTR_ITALIC        = 0x0002;
const sal_uInt16 EXC_FONTATTR_UNDERLINE     = 0x0004;
const sal_uInt16 EXC_FONTATTR_STRIKEOUT     = 0x0008;
const sal_uInt16 EXC_FONTATTR_OUTLINE       = 0x0010;
const sal_uInt16 EXC_FONTATTR_SHADOW        = 0x0020;

// weight
const sal_uInt16 EXC_FONTWGHT_DONTKNOW      = 0;
const sal_uInt16 EXC_FONTWGHT_THIN          = 100;
const sal_uInt16 EXC_FONTWGHT_ULTRALIGHT    = 200;
const sal_uInt16 EXC_FONTWGHT_LIGHT         = 300;
const sal_uInt16 EXC_FONTWGHT_SEMILIGHT     = 350;
const sal_uInt16 EXC_FONTWGHT_NORMAL        = 400;
const sal_uInt16 EXC_FONTWGHT_MEDIUM        = 500;
const sal_uInt16 EXC_FONTWGHT_SEMIBOLD      = 600;
const sal_uInt16 EXC_FONTWGHT_BOLD          = 700;
const sal_uInt16 EXC_FONTWGHT_ULTRABOLD     = 800;
const sal_uInt16 EXC_FONTWGHT_BLACK         = 900;

// underline
const sal_uInt8 EXC_FONTUNDERL_NONE         = 0x00;
const sal_uInt8 EXC_FONTUNDERL_SINGLE       = 0x01;
const sal_uInt8 EXC_FONTUNDERL_DOUBLE       = 0x02;
const sal_uInt8 EXC_FONTUNDERL_SINGLE_ACC   = 0x21;
const sal_uInt8 EXC_FONTUNDERL_DOUBLE_ACC   = 0x22;

// escapement
const sal_uInt16 EXC_FONTESC_NONE           = 0x00;
const sal_uInt16 EXC_FONTESC_SUPER          = 0x01;
const sal_uInt16 EXC_FONTESC_SUB            = 0x02;


// (0x0043, 0x0243, 0x0443, 0x00E0) XF ----------------------------------------

const sal_uInt16 EXC_ID2_XF                 = 0x0043;
const sal_uInt16 EXC_ID3_XF                 = 0x0243;
const sal_uInt16 EXC_ID4_XF                 = 0x0443;
const sal_uInt16 EXC_ID_XF                  = 0x00E0;

const sal_uInt32 EXC_XF_MAXCOUNT            = 4050;     /// Maximum number of all XF records.
const sal_uInt32 EXC_XF_MAXSTYLECOUNT       = 1536;     /// Arbitrary maximum number of style XFs.
const sal_uInt16 EXC_XF_USEROFFSET          = 23;       /// Index to first user defined record.
const sal_uInt16 EXC_XF_DEFAULTSTYLE        = 0;        /// Excel index to default style XF.
const sal_uInt16 EXC_XF_DEFAULTCELL         = 15;       /// Excel index to default cell XF.

const sal_uInt16 EXC_XF_LOCKED              = 0x0001;
const sal_uInt16 EXC_XF_HIDDEN              = 0x0002;
const sal_uInt16 EXC_XF_STYLE               = 0x0004;
const sal_uInt16 EXC_XF_STYLEPARENT         = 0x0FFF;   /// Syles don't have a parent.
const sal_uInt16 EXC_XF_WRAPPED             = 0x0008;   /// Automatic line break.
const sal_uInt16 EXC_XF_SHRINK              = 0x0010;   /// Shrink to fit into cell.

const sal_uInt8 EXC_XF_DIFF_VALFMT          = 0x01;
const sal_uInt8 EXC_XF_DIFF_FONT            = 0x02;
const sal_uInt8 EXC_XF_DIFF_ALIGN           = 0x04;
const sal_uInt8 EXC_XF_DIFF_BORDER          = 0x08;
const sal_uInt8 EXC_XF_DIFF_AREA            = 0x10;
const sal_uInt8 EXC_XF_DIFF_PROT            = 0x20;

const sal_uInt8 EXC_XF2_VALFMT_MASK         = 0x3F;
const sal_uInt8 EXC_XF2_LOCKED              = 0x40;
const sal_uInt8 EXC_XF2_HIDDEN              = 0x80;
const sal_uInt8 EXC_XF2_LEFTLINE            = 0x08;
const sal_uInt8 EXC_XF2_RIGHTLINE           = 0x10;
const sal_uInt8 EXC_XF2_TOPLINE             = 0x20;
const sal_uInt8 EXC_XF2_BOTTOMLINE          = 0x40;
const sal_uInt8 EXC_XF2_BACKGROUND          = 0x80;

const sal_uInt16 EXC_XF8_SHRINKTOFIT        = 0x0010;
const sal_uInt16 EXC_XF8_MERGE              = 0x0020;

const sal_uInt8 EXC_XF8_STACKED             = 0xFF;     /// Special rotation angle.

// Diagonal Border Line Styles
const sal_uInt32 EXC_XF_DIAGONAL_TR_TO_BL   = 0x40000000;   /// Top Right to Bottom Left.
const sal_uInt32 EXC_XF_DIAGONAL_BL_TO_TR   = 0x80000000;   /// Bottom Left to Top Right.
const sal_uInt32 EXC_XF_DIAGONAL_BOTH       = 0xC0000000;   /// Both.

/** Horizontal alignment of cell contents. */
enum XclHorAlign
{
    xlHAlignGeneral                         = 0x00,
    xlHAlignLeft                            = 0x01,
    xlHAlignCenter                          = 0x02,
    xlHAlignRight                           = 0x03,
    xlHAlignFill                            = 0x04,
    xlHAlignJustify                         = 0x05,
    xlHAlignCenterAcrSel                    = 0x06,
    xlHAlignDistrib                         = 0x07,
    xlHAlign_Default                        = xlHAlignGeneral

};

/** Vertical alignment of cell contents. */
enum XclVerAlign
{
    xlVAlignTop                             = 0x00,
    xlVAlignCenter                          = 0x01,
    xlVAlignBottom                          = 0x02,
    xlVAlignJustify                         = 0x03,
    xlVAlignDistrib                         = 0x04,
    xlVAlign_Default                        = xlVAlignBottom
};

/** Text orientation. */
enum XclTextOrient
{
    xlTextOrientNoRot                       = 0x00,
    xlTextOrientTopBottom                   = 0x01,
    xlTextOrient90ccw                       = 0x02,
    xlTextOrient90cw                        = 0x03,
    xlTextOrient_Default                    = xlTextOrientNoRot
};

/** CTL text direction. */
enum XclTextDirection
{
    xlTextDirContext                        = 0x00,
    xlTextDirLTR                            = 0x01,
    xlTextDirRTL                            = 0x02,
    xlTextDir_Default                       = xlTextDirContext
};


// (0x0092) PALETTE -----------------------------------------------------------

const sal_uInt16 EXC_ID_PALETTE             = 0x0092;

const sal_uInt16 EXC_COLOR_BIFF2_BLACK      = 0;
const sal_uInt16 EXC_COLOR_BIFF2_WHITE      = 1;

const sal_uInt16 EXC_COLOR_USEROFFSET       = 8;        ///First user defined color.
const sal_uInt16 EXC_COLOR_WINDOWTEXT3      = 24;       /// System window text color (BIFF3-BIFF4).
const sal_uInt16 EXC_COLOR_WINDOWBACK3      = 25;       /// System window background color (BIFF3-BIFF4).
const sal_uInt16 EXC_COLOR_WINDOWTEXT       = 64;       /// System window text color (>=BIFF5).
const sal_uInt16 EXC_COLOR_WINDOWBACK       = 65;       /// System window background color (>=BIFF5).
const sal_uInt16 EXC_COLOR_FONTAUTO         = 0x7FFF;   /// Font auto color (system window text color).

// (0x0293) STYLE -------------------------------------------------------------

const sal_uInt16 EXC_ID_STYLE               = 0x0293;

const sal_uInt16 EXC_STYLE_BUILTIN          = 0x8000;
const sal_uInt16 EXC_STYLE_XFMASK           = 0x0FFF;

const sal_uInt8 EXC_STYLE_NORMAL            = 0x00;         /// "Normal" style.
const sal_uInt8 EXC_STYLE_ROWLEVEL          = 0x01;         /// "RowLevel_*" styles.
const sal_uInt8 EXC_STYLE_COLLEVEL          = 0x02;         /// "ColLevel_*" styles.
const sal_uInt8 EXC_STYLE_COMMA             = 0x03;         /// "Comma" style.
const sal_uInt8 EXC_STYLE_CURRENCY          = 0x04;         /// "Currency" style.
const sal_uInt8 EXC_STYLE_PERCENT           = 0x05;         /// "Percent" style.
const sal_uInt8 EXC_STYLE_COMMA_0           = 0x06;         /// "Comma [0]" style.
const sal_uInt8 EXC_STYLE_CURRENCY_0        = 0x07;         /// "Currency [0]" style.
const sal_uInt8 EXC_STYLE_HYPERLINK         = 0x08;         /// "Hyperlink" style.
const sal_uInt8 EXC_STYLE_FOLLOWED_HYPERLINK= 0x09;         /// "Followed_Hyperlink" style.
const sal_uInt8 EXC_STYLE_USERDEF           = 0xFF;         /// No built-in style.

const sal_uInt8 EXC_STYLE_LEVELCOUNT        = 7;            /// Number of outline level styles.
const sal_uInt8 EXC_STYLE_NOLEVEL           = 0xFF;         /// Default value for unused level.


// ============================================================================

// Color data =================================================================

/** Stores all default colors for a specific BIFF version. */
class XclDefaultPalette
{
public:
    explicit                    XclDefaultPalette( XclBiff eBiff = xlBiffUnknown );

    /** Activates the default colors for the passed BIFF version. */
    void                        SetDefaultColors( XclBiff eBiff );

    /** Returns the color count in the current palette. */
    inline sal_uInt32           GetColorCount() const { return mnTableSize - EXC_COLOR_USEROFFSET; }

    /** Returns the default RGB color data for a (non-zero-based) Excel color or COL_AUTO on error. */
    ColorData                   GetDefColorData( sal_uInt16 nXclIndex ) const;
    /** Returns the default color for a (non-zero-based) Excel color or COL_AUTO on error. */
    inline Color                GetDefColor( sal_uInt16 nXclIndex ) const
                                    { return Color( GetDefColorData( nXclIndex ) ); }

private:
    const ColorData*            mpColorTable;       /// The table with RGB values.
    ColorData                   mnWindowText;       /// System window text color.
    ColorData                   mnWindowBack;       /// System window background color.
    sal_uInt32                  mnTableSize;        /// The color table size.
};


// Font data ==================================================================

class Font;
class SvxFont;

/** This struct helps reading and writing Excel fonts.
    @descr  It stores all Excel compatible properties of a font. In detail this is the
    name, family, character set, height, color, boldness, posture, script, underline,
    strikeout, outline and shadow of the font. */
struct XclFontData
{
    String                      maName;         /// Font name.
    String                      maStyle;        /// String with styles (bold, italic).
    sal_uInt16                  mnHeight;       /// Font height in twips (1/20 of a point).
    sal_uInt16                  mnColor;        /// Index to color palette.
    sal_uInt16                  mnWeight;       /// Boldness: 400=normal, 700=bold.
    sal_uInt16                  mnEscapem;      /// Escapement type.
    sal_uInt8                   mnFamily;       /// Windows font family.
    sal_uInt8                   mnCharSet;      /// Windows character set.
    sal_uInt8                   mnUnderline;    /// Underline style.
    bool                        mbItalic;       /// true = Italic.
    bool                        mbStrikeout;    /// true = Struck out.
    bool                        mbOutline;      /// true = Outlined.
    bool                        mbShadow;       /// true = Shadowed.

    /** Constructs an empty font data structure. */
    explicit                    XclFontData();
    /** Constructs a font data structure and fills it with the passed font attributes (except color). */
    explicit                    XclFontData( const Font& rFont );
    /** as directly above but also fills in the escapement member. */
    explicit                    XclFontData( const SvxFont& rFont );

    /** Resets all members to default (empty) values. */
    void                        Clear();
    /** Fills all members (except color and escapement) from the passed font. */
    void                        FillFromFont( const Font& rFont );
    /** Fills all members (except color) from the passed SVX font. */
    void                        FillFromSvxFont( const SvxFont& rFont );

// *** conversion of VCL/SVX constants *** ------------------------------------

    /** Returns the Calc font family. */
    FontFamily                  GetScFamily( CharSet eDefCharSet ) const;
    /** Returns the Calc font character set. */
    CharSet                     GetScCharSet() const;
    /** Returns the Calc font posture. */
    FontItalic                  GetScPosture() const;
    /** Returns the Calc font weight. */
    FontWeight                  GetScWeight() const;
    /** Returns the Calc font underline style. */
    FontUnderline               GetScUnderline() const;
    /** Returns the Calc escapement style. */
    SvxEscapement               GetScEscapement() const;
    /** Returns the Calc strike-out style. */
    FontStrikeout               GetScStrikeout() const;

    /** Sets the Calc font height (in twips). */
    void                        SetScHeight( sal_Int32 nTwips );
    /** Sets the Calc font family. */
    void                        SetScFamily( FontFamily eScFamily );
    /** Sets the Calc character set. */
    void                        SetScCharSet( CharSet eScCharSet );
    /** Sets the Calc font posture. */
    void                        SetScPosture( FontItalic eScPosture );
    /** Sets the Calc font weight. */
    void                        SetScWeight( FontWeight eScWeight );
    /** Sets the Calc underline style. */
    void                        SetScUnderline( FontUnderline eScUnderl );
    /** Sets the Calc escapement style. */
    void                        SetScEscapement( short nScEscapem );
    /** Sets the Calc strike-out style. */
    void                        SetScStrikeout( FontStrikeout eScStrikeout );

// *** conversion of API constants *** ----------------------------------------

    /** Returns the API font height. */
    float                       GetApiHeight() const;
    /** Returns the API font family. */
    sal_Int16                   GetApiFamily() const;
    /** Returns the API character set. */
    sal_Int16                   GetApiCharSet() const;
    /** Returns the API font posture. */
    ::com::sun::star::awt::FontSlant GetApiPosture() const;
    /** Returns the API font weight. */
    float                       GetApiWeight() const;
    /** Returns the API font underline style. */
    sal_Int16                   GetApiUnderline() const;
    /** Returns the API font strike-out style. */
    sal_Int16                   GetApiStrikeout() const;

    /** Sets the API font height. */
    void                        SetApiHeight( float fPoint );
    /** Sets the API font family. */
    void                        SetApiFamily( sal_Int16 nApiFamily );
    /** Sets the API character set. */
    void                        SetApiCharSet( sal_Int16 nApiCharSet );
    /** Sets the API font posture. */
    void                        SetApiPosture( ::com::sun::star::awt::FontSlant eApiPosture );
    /** Sets the API font weight. */
    void                        SetApiWeight( float fApiWeight );
    /** Sets the API font underline style. */
    void                        SetApiUnderline( sal_Int16 nApiUnderl );
    /** Sets the API font strike-out style. */
    void                        SetApiStrikeout( sal_Int16 nApiStrikeout );
};

bool operator==( const XclFontData& rLeft, const XclFontData& rRight );


// Cell formatting data (XF) ==================================================

/** Contains all cell protection attributes. */
struct XclCellProt
{
    bool                        mbLocked;       /// true = Locked against editing.
    bool                        mbHidden;       /// true = Formula is hidden.

    explicit                    XclCellProt();
};

bool operator==( const XclCellProt& rLeft, const XclCellProt& rRight );


// ----------------------------------------------------------------------------

/** Contains all cell alignment attributes. */
struct XclCellAlign
{
    XclHorAlign                 meHorAlign;     /// Horizontal alignment.
    XclVerAlign                 meVerAlign;     /// Vertical alignment.
    XclTextDirection            meTextDir;      /// CTL text direction.
    XclTextOrient               meOrient;       /// Text orientation.
    sal_uInt8                   mnRotation;     /// Text rotation angle.
    sal_uInt8                   mnIndent;       /// Indentation.
    bool                        mbWrapped;      /// true = Multi-line text.

    explicit                    XclCellAlign();
};

bool operator==( const XclCellAlign& rLeft, const XclCellAlign& rRight );


// ----------------------------------------------------------------------------

/** Contains color and line style for each cell border line. */
struct XclCellBorder
{
    sal_uInt16                  mnLeftColor;    /// Palette index for left line.
    sal_uInt16                  mnRightColor;   /// Palette index for right line.
    sal_uInt16                  mnTopColor;     /// Palette index for top line.
    sal_uInt16                  mnBottomColor;  /// Palette index for bottom line.
    sal_uInt8                   mnLeftLine;     /// Style of left line.
    sal_uInt8                   mnRightLine;    /// Style of right line.
    sal_uInt8                   mnTopLine;      /// Style of top line.
    sal_uInt8                   mnBottomLine;   /// Style of bottom line.

    explicit                    XclCellBorder();
};

bool operator==( const XclCellBorder& rLeft, const XclCellBorder& rRight );


// ----------------------------------------------------------------------------

/** Contains background colors and pattern for a cell. */
struct XclCellArea
{
    sal_uInt16                  mnForeColor;    /// Palette index to foreground color.
    sal_uInt16                  mnBackColor;    /// Palette index to background color.
    sal_uInt8                   mnPattern;      /// Fill pattern.

    explicit                    XclCellArea();

    /** Returns true, if the area represents transparent state. */
    bool                        IsTransparent() const;
};

bool operator==( const XclCellArea& rLeft, const XclCellArea& rRight );


// ----------------------------------------------------------------------------

/** Contains base members for XF record import/export.
    @In detail this class stores the XF type (cell/style), the index to the parent
    style XF and all "attribute used" flags, which reflect the state of specific
    attribute groups (true = user has changed the attributes). */
class XclXFBase
{
public:
    explicit                    XclXFBase( bool bCellXF );

    /** Sets all "attribute used" flags to the passed state. */
    void                        SetAllUsedFlags( bool bUsed );

    inline bool                 IsCellXF() const    { return mbCellXF; }
    inline bool                 IsStyleXF() const   { return !IsCellXF(); }

protected:
    /** Returns true, if this object is equal to the passed. */
    bool                        Equals( const XclXFBase& rCmp ) const;

protected:
    sal_uInt16                  mnParent;           /// Index to parent style XF.
    bool                        mbCellXF;           /// true = cell XF, false = style XF.
    bool                        mbProtUsed;         /// true = cell protection used.
    bool                        mbFontUsed;         /// true = font index used.
    bool                        mbFmtUsed;          /// true = number format used.
    bool                        mbAlignUsed;        /// true = alignment used.
    bool                        mbBorderUsed;       /// true = border data used.
    bool                        mbAreaUsed;         /// true = area data used.
};


// ============================================================================

#endif

