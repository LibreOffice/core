/*************************************************************************
 *
 *  $RCSfile: xlstyle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:14 $
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
    sal_uInt8                   mnFamily;       /// Font family.
    sal_uInt8                   mnCharSet;      /// Character set.
    bool                        mbItalic;       /// true = Italic.
    bool                        mbStrikeout;    /// true = Struck out.
    bool                        mbOutline;      /// true = Outlined.
    bool                        mbShadow;       /// true = Shadowed.

    inline explicit             XclFontData() { Clear(); }

    /** Resets all members to default (empty) values. */
    void                        Clear();
};


// Style (XF) data ============================================================

/** Horizontal alignment of cell contents. */
enum XclHorAlign
{
    xlHAlignGeneral                 = 0x00,
    xlHAlignLeft                    = 0x01,
    xlHAlignCenter                  = 0x02,
    xlHAlignRight                   = 0x03,
    xlHAlignFill                    = 0x04,
    xlHAlignJustify                 = 0x05,
    xlHAlignCenterAcrSel            = 0x06,
    xlHAlignDistrib                 = 0x07
};

/** Vertical alignment of cell contents. */
enum XclVerAlign
{
    xlVAlignTop                     = 0x00,
    xlVAlignCenter                  = 0x01,
    xlVAlignBottom                  = 0x02,
    xlVAlignJustify                 = 0x03,
    xlVAlignDistrib                 = 0x04
};

/** Text orientation. */
enum XclTextOrient
{
    xlTextOrientNoRot               = 0x00,
    xlTextOrientTopBottom           = 0x01,
    xlTextOrient90ccw               = 0x02,
    xlTextOrient90cw                = 0x03,
    xlTextOrientRot                 = 0x04
};

/** CTL text direction. */
enum XclTextDirection
{
    xlTextDirContext                = 0x00,
    xlTextDirLTR                    = 0x01,
    xlTextDirRTL                    = 0x02
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

