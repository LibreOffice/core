/*************************************************************************
 *
 *  $RCSfile: xlstyle.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-24 11:56:52 $
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


// Color data =================================================================

/** Stores all default colors for a specific BIFF version. */
class XclDefaultPalette
{
private:
    const ColorData*            mpColorTable;       /// The table with RGB values.
    sal_uInt32                  mnTableSize;        /// The color table size.
    sal_uInt16                  mnIndexOffset;      /// The Excel index of the first color.

public:
    explicit                    XclDefaultPalette( XclBiff eBiff = xlBiffUnknown );

    /** Activates the default colors for the passed BIFF version. */
    void                        SetDefaultColors( XclBiff eBiff );

    /** Returns the color count in the current palette. */
    inline sal_uInt32           GetColorCount() const { return mnTableSize; }

    /** Returns the Excel index of the first color. */
    inline sal_uInt32           GetIndexOffset() const { return mnIndexOffset; }
    /** Returns the Excel index of a 0-based color index. */
    inline sal_uInt16           GetXclIndex( sal_uInt32 nIndex ) const;

    /** Returns the default RGB color data for a (non-zero-based) Excel color or nDefault on error. */
    ColorData                   GetDefColorData( sal_uInt16 nXclIndex, ColorData nDefault = COL_AUTO ) const;
    /** Returns the default color for a (non-zero-based) Excel color or nDefault on error. */
    inline Color                GetDefColor( sal_uInt16 nXclIndex, ColorData nDefault = COL_AUTO ) const;
};

inline sal_uInt16 XclDefaultPalette::GetXclIndex( sal_uInt32 nIndex ) const
{
    return static_cast< sal_uInt16 >( nIndex + GetIndexOffset() );
}

inline Color XclDefaultPalette::GetDefColor( sal_uInt16 nXclIndex, ColorData nDefault ) const
{
    return Color( GetDefColorData( nXclIndex, nDefault ) );
}


// Font data ==================================================================

/** Text underline style. */
enum XclUnderline
{
    xlUnderlNone                    = 0x00,
    xlUnderlSingle                  = 0x01,
    xlUnderlDouble                  = 0x02,
    xlUnderlSingleAcc               = 0x21,
    xlUnderlDoubleAcc               = 0x22
};

/** Super-/subscript type. */
enum XclEscapement
{
    xlEscNone                       = 0x00,
    xlEscSuper                      = 0x01,
    xlEscSub                        = 0x02
};


// ----------------------------------------------------------------------------

class Font;

/** This struct helps reading and writing Excel fonts.
    @descr  It stores all Excel compatible properties of a font. In detail this is the
    name, family, character set, height, color, boldness, posture, script, underline,
    strikeout, outline and shadow of the font. */
struct XclFontData
{
    String                      maName;         /// Font name.
    String                      maStyle;        /// String with styles (bold, italic).
    XclUnderline                meUnderline;    /// Underline style.
    XclEscapement               meEscapem;      /// Super-/subscript.
    sal_uInt16                  mnHeight;       /// Font height in twips (1/20 of a point).
    sal_uInt16                  mnColor;        /// Index to color palette.
    sal_uInt16                  mnWeight;       /// Boldness: 400=normal, 700=bold.
    sal_uInt8                   mnFamily;       /// Windows font family.
    sal_uInt8                   mnCharSet;      /// Windows character set.
    bool                        mbItalic;       /// true = Italic.
    bool                        mbStrikeout;    /// true = Struck out.
    bool                        mbOutline;      /// true = Outlined.
    bool                        mbShadow;       /// true = Shadowed.

    /** Constructs an empty font data structure. */
    explicit                    XclFontData();
    /** Constructs a font data structure and fills it with the passed font attributes (except color). */
    explicit                    XclFontData( const Font& rFont );

    /** Resets all members to default (empty) values. */
    void                        Clear();
    /** Fills all members (except color) from the passed font. */
    void                        FillFromFont( const Font& rFont );

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
    void                        SetScEscapement( SvxEscapement eScEscapem );
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

/** Horizontal alignment of cell contents. */
enum XclHorAlign
{
    xlHAlignGeneral             = 0x00,
    xlHAlignLeft                = 0x01,
    xlHAlignCenter              = 0x02,
    xlHAlignRight               = 0x03,
    xlHAlignFill                = 0x04,
    xlHAlignJustify             = 0x05,
    xlHAlignCenterAcrSel        = 0x06,
    xlHAlignDistrib             = 0x07,
    xlHAlign_Default            = xlHAlignGeneral

};

/** Vertical alignment of cell contents. */
enum XclVerAlign
{
    xlVAlignTop                 = 0x00,
    xlVAlignCenter              = 0x01,
    xlVAlignBottom              = 0x02,
    xlVAlignJustify             = 0x03,
    xlVAlignDistrib             = 0x04,
    xlVAlign_Default            = xlVAlignBottom
};

/** Text orientation. */
enum XclTextOrient
{
    xlTextOrientNoRot           = 0x00,
    xlTextOrientTopBottom       = 0x01,
    xlTextOrient90ccw           = 0x02,
    xlTextOrient90cw            = 0x03,
    xlTextOrient_Default        = xlTextOrientNoRot
};

/** CTL text direction. */
enum XclTextDirection
{
    xlTextDirContext            = 0x00,
    xlTextDirLTR                = 0x01,
    xlTextDirRTL                = 0x02,
    xlTextDir_Default           = xlTextDirContext
};

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
protected:
    sal_uInt16                  mnParent;           /// Index to parent style XF.
    bool                        mbCellXF;           /// true = cell XF, false = style XF.
    bool                        mbProtUsed;         /// true = cell protection used.
    bool                        mbFontUsed;         /// true = font index used.
    bool                        mbFmtUsed;          /// true = number format used.
    bool                        mbAlignUsed;        /// true = alignment used.
    bool                        mbBorderUsed;       /// true = border data used.
    bool                        mbAreaUsed;         /// true = area data used.

public:
    explicit                    XclXFBase( bool bCellXF );

    /** Sets all "attribute used" flags to the passed state. */
    void                        SetAllUsedFlags( bool bUsed );

    inline bool                 IsCellXF() const    { return mbCellXF; }
    inline bool                 IsStyleXF() const   { return !IsCellXF(); }

protected:
    /** Returns true, if this object is equal to the passed. */
    bool                        Equals( const XclXFBase& rCmp ) const;
};


// Page format ================================================================

/** The type of a margin value. */
enum XclMarginType
{
    xlLeftMargin,
    xlRightMargin,
    xlTopMargin,
    xlBottomMargin
};

/** Orientation for page breaks. */
enum XclPBOrientation
{
    xlPBHorizontal,
    xlPBVertical
};


// ============================================================================

#endif

