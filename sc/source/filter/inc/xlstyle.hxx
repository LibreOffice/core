/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XLSTYLE_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XLSTYLE_HXX

#include <map>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/frmdir.hxx>
#include <svl/zforlist.hxx>
#include "fapihelper.hxx"

class XclRoot;

// Constants and Enumerations =================================================

// Line styles ----------------------------------------------------------------

const sal_uInt8 EXC_LINE_NONE                 = 0x00;
const sal_uInt8 EXC_LINE_THIN                 = 0x01;
const sal_uInt8 EXC_LINE_MEDIUM               = 0x02;
const sal_uInt8 EXC_LINE_DASHED               = 0x03;
const sal_uInt8 EXC_LINE_DOTTED               = 0x04;
const sal_uInt8 EXC_LINE_THICK                = 0x05;
const sal_uInt8 EXC_LINE_DOUBLE               = 0x06;
const sal_uInt8 EXC_LINE_HAIR                 = 0x07;
const sal_uInt8 EXC_LINE_MEDIUM_DASHED        = 0x08;
const sal_uInt8 EXC_LINE_THIN_DASHDOT         = 0x09;
const sal_uInt8 EXC_LINE_MEDIUM_DASHDOT       = 0x0A;
const sal_uInt8 EXC_LINE_THIN_DASHDOTDOT      = 0x0B;
const sal_uInt8 EXC_LINE_MEDIUM_DASHDOTDOT    = 0x0C;
const sal_uInt8 EXC_LINE_MEDIUM_SLANT_DASHDOT = 0x0D;

// Background patterns --------------------------------------------------------

const sal_uInt8 EXC_PATT_NONE               = 0x00;
const sal_uInt8 EXC_PATT_SOLID              = 0x01;
const sal_uInt8 EXC_PATT_50_PERC            = 0x02;
const sal_uInt8 EXC_PATT_75_PERC            = 0x03;
const sal_uInt8 EXC_PATT_25_PERC            = 0x04;
const sal_uInt8 EXC_PATT_12_5_PERC          = 0x11;
const sal_uInt8 EXC_PATT_6_25_PERC          = 0x12;

// (0x001E, 0x041E) FORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID2_FORMAT             = 0x001E;
const sal_uInt16 EXC_ID4_FORMAT             = 0x041E;

const sal_uInt16 EXC_FORMAT_OFFSET5         = 164;
const sal_uInt16 EXC_FORMAT_OFFSET8         = 164;
const sal_uInt16 EXC_FORMAT_NOTFOUND        = 0xFFFF;

// (0x0031) FONT --------------------------------------------------------------

const sal_uInt16 EXC_ID2_FONT               = 0x0031;
const sal_uInt16 EXC_ID3_FONT               = 0x0231;

const sal_uInt16 EXC_FONT_APP               = 0;        /// Application font index.
const sal_uInt16 EXC_FONT_NOTFOUND          = 0xFFFF;

const size_t EXC_FONT_MAXCOUNT4             = 0x00FF;
const size_t EXC_FONT_MAXCOUNT5             = 0x00FF;
const size_t EXC_FONT_MAXCOUNT8             = 0xFFFF;

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
const sal_uInt16 EXC_ID5_XF                 = 0x00E0;

const sal_uInt32 EXC_XF_MAXCOUNT            = 4050;     /// Maximum number of all XF records.
const sal_uInt32 EXC_XF_MAXSTYLECOUNT       = 1536;     /// Arbitrary maximum number of style XFs.
const sal_uInt16 EXC_XF_DEFAULTSTYLE        = 0;        /// Excel index to default style XF.
const sal_uInt16 EXC_XF_DEFAULTCELL         = 15;       /// Excel index to default cell XF.
const sal_uInt16 EXC_XF_NOTFOUND            = 0xFFFF;   /// Special index for "not found" state.

const sal_uInt32 EXC_XFID_NOTFOUND          = 0xFFFFFFFF;

const sal_uInt16 EXC_XF_LOCKED              = 0x0001;
const sal_uInt16 EXC_XF_HIDDEN              = 0x0002;
const sal_uInt16 EXC_XF_STYLE               = 0x0004;
const sal_uInt16 EXC_XF_STYLEPARENT         = 0x0FFF;   /// Styles don't have a parent.
const sal_uInt16 EXC_XF_LINEBREAK           = 0x0008;   /// Automatic line break.
const sal_uInt16 EXC_XF_SHRINK              = 0x0010;   /// Shrink to fit into cell.

const sal_uInt8 EXC_XF_DIFF_VALFMT          = 0x01;
const sal_uInt8 EXC_XF_DIFF_FONT            = 0x02;
const sal_uInt8 EXC_XF_DIFF_ALIGN           = 0x04;
const sal_uInt8 EXC_XF_DIFF_BORDER          = 0x08;
const sal_uInt8 EXC_XF_DIFF_AREA            = 0x10;
const sal_uInt8 EXC_XF_DIFF_PROT            = 0x20;

const sal_uInt8 EXC_XF_HOR_GENERAL          = 0x00;
const sal_uInt8 EXC_XF_HOR_LEFT             = 0x01;
const sal_uInt8 EXC_XF_HOR_CENTER           = 0x02;
const sal_uInt8 EXC_XF_HOR_RIGHT            = 0x03;
const sal_uInt8 EXC_XF_HOR_FILL             = 0x04;
const sal_uInt8 EXC_XF_HOR_JUSTIFY          = 0x05;
const sal_uInt8 EXC_XF_HOR_CENTER_AS        = 0x06;
const sal_uInt8 EXC_XF_HOR_DISTRIB          = 0x07;

const sal_uInt8 EXC_XF_VER_TOP              = 0x00;
const sal_uInt8 EXC_XF_VER_CENTER           = 0x01;
const sal_uInt8 EXC_XF_VER_BOTTOM           = 0x02;
const sal_uInt8 EXC_XF_VER_JUSTIFY          = 0x03;
const sal_uInt8 EXC_XF_VER_DISTRIB          = 0x04;

const sal_uInt8 EXC_XF_TEXTDIR_CONTEXT      = 0x00;
const sal_uInt8 EXC_XF_TEXTDIR_LTR          = 0x01;
const sal_uInt8 EXC_XF_TEXTDIR_RTL          = 0x02;

const sal_uInt8 EXC_XF2_VALFMT_MASK         = 0x3F;
const sal_uInt8 EXC_XF2_LOCKED              = 0x40;
const sal_uInt8 EXC_XF2_HIDDEN              = 0x80;
const sal_uInt8 EXC_XF2_LEFTLINE            = 0x08;
const sal_uInt8 EXC_XF2_RIGHTLINE           = 0x10;
const sal_uInt8 EXC_XF2_TOPLINE             = 0x20;
const sal_uInt8 EXC_XF2_BOTTOMLINE          = 0x40;
const sal_uInt8 EXC_XF2_BACKGROUND          = 0x80;

const sal_uInt16 EXC_XF8_SHRINK             = 0x0010;   /// Shrink to fit into cell.
const sal_uInt16 EXC_XF8_MERGE              = 0x0020;

const sal_uInt32 EXC_XF_DIAGONAL_TL_TO_BR   = 0x40000000;   /// Top-left to bottom-right.
const sal_uInt32 EXC_XF_DIAGONAL_BL_TO_TR   = 0x80000000;   /// Bottom-left to top-right.
const sal_uInt32 EXC_XF_DIAGONAL_BOTH       = 0xC0000000;   /// Both.

// (0x0045) EFONT -------------------------------------------------------------

const sal_uInt16 EXC_ID_EFONT               = 0x0045;

// (0x0092) PALETTE -----------------------------------------------------------

const sal_uInt16 EXC_ID_PALETTE             = 0x0092;

const sal_uInt16 EXC_COLOR_BIFF2_BLACK      = 0;
const sal_uInt16 EXC_COLOR_BIFF2_WHITE      = 1;

const sal_uInt16 EXC_COLOR_USEROFFSET       = 8;        /// First user defined color.
const sal_uInt16 EXC_COLOR_WINDOWTEXT3      = 24;       /// System window text color (BIFF3-BIFF4).
const sal_uInt16 EXC_COLOR_WINDOWBACK3      = 25;       /// System window background color (BIFF3-BIFF4).
const sal_uInt16 EXC_COLOR_WINDOWTEXT       = 64;       /// System window text color (>=BIFF5).
const sal_uInt16 EXC_COLOR_WINDOWBACK       = 65;       /// System window background color (>=BIFF5).
const sal_uInt16 EXC_COLOR_BUTTONBACK       = 67;       /// System button background color (face color).
const sal_uInt16 EXC_COLOR_CHWINDOWTEXT     = 77;       /// System window text color (BIFF8 charts).
const sal_uInt16 EXC_COLOR_CHWINDOWBACK     = 78;       /// System window background color (BIFF8 charts).
const sal_uInt16 EXC_COLOR_CHBORDERAUTO     = 79;       /// Automatic frame border for series (BIFF8 charts).
const sal_uInt16 EXC_COLOR_NOTEBACK         = 80;       /// Note background color.
const sal_uInt16 EXC_COLOR_NOTETEXT         = 81;       /// Note text color.
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

// (0x0892) STYLEEXT ----------------------------------------------------------

const sal_uInt16 EXC_ID_STYLEEXT            = 0x0892;

const sal_uInt8 EXC_STYLEEXT_BUILTIN        = 0x01;
const sal_uInt8 EXC_STYLEEXT_HIDDEN         = 0x02;
const sal_uInt8 EXC_STYLEEXT_CUSTOM         = 0x04;

// Color data =================================================================

/** Stores all default colors for a specific BIFF version. */
class XclDefaultPalette
{
public:
    explicit            XclDefaultPalette( const XclRoot& rRoot );

    /** Returns the color count in the current palette. */
    sal_uInt32   GetColorCount() const { return mnTableSize - EXC_COLOR_USEROFFSET; }

    /** Returns the default color for a (non-zero-based) Excel color or COL_AUTO on error. */
    Color        GetDefColor( sal_uInt16 nXclIndex ) const;

    /** Returns true, if the passed Excel color index is a system color. */
    bool         IsSystemColor( sal_uInt16 nXclIndex ) const { return nXclIndex >= mnTableSize; }

private:
    const Color*        mpnColorTable;      /// The table with RGB values.
    Color               mnWindowText;       /// System window text color.
    Color               mnWindowBack;       /// System window background color.
    Color               mnFaceColor;        /// System button background color.
    Color               mnNoteText;         /// Note text color.
    Color               mnNoteBack;         /// Note background color.
    sal_uInt32          mnTableSize;        /// The color table size.
};

// Font data ==================================================================

namespace vcl { class Font; }
class SvxFont;

/** This struct helps reading and writing Excel fonts.

    It stores all Excel compatible properties of a font. In detail this is the
    name, family, character set, height, color, boldness, posture, script,
    underline, strikeout, outline and shadow of the font.
 */
struct XclFontData
{
    OUString            maName;         /// Font name.
    OUString            maStyle;        /// String with styles (bold, italic).
    Color               maColor;        /// Font color.
    sal_uInt16          mnHeight;       /// Font height in twips (1/20 of a point).
    sal_uInt16          mnWeight;       /// Boldness: 400=normal, 700=bold.
    sal_uInt16          mnEscapem;      /// Escapement type.
    sal_uInt8           mnFamily;       /// Windows font family.
    sal_uInt8           mnCharSet;      /// Windows character set.
    sal_uInt8           mnUnderline;    /// Underline style.
    bool                mbItalic;       /// true = Italic.
    bool                mbStrikeout;    /// true = Struck out.
    bool                mbOutline;      /// true = Outlined.
    bool                mbShadow;       /// true = Shadowed.

    /** Constructs an empty font data structure. */
    explicit            XclFontData();
    /** Constructs a font data structure and fills it with the passed font attributes (except color). */
    explicit            XclFontData( const vcl::Font& rFont );
    /** As directly above but also fills in the escapement member. */
    explicit            XclFontData( const SvxFont& rFont );

    /** Resets all members to default (empty) values. */
    void                Clear();
    /** Fills all members (except color and escapement) from the passed font. */
    void                FillFromVclFont( const vcl::Font& rFont );
    /** Fills all members (except color) from the passed SVX font. */
    void                FillFromSvxFont( const SvxFont& rFont );

// *** conversion of VCL/SVX constants *** ------------------------------------

    /** Returns the Calc font family. */
    FontFamily          GetScFamily( rtl_TextEncoding eDefTextEnc ) const;
    /** Returns the font text encoding. */
    rtl_TextEncoding    GetFontEncoding() const;
    /** Returns the Calc font posture. */
    FontItalic          GetScPosture() const;
    /** Returns the Calc font weight. */
    FontWeight          GetScWeight() const;
    /** Returns the Calc font underline style. */
    FontLineStyle       GetScUnderline() const;
    /** Returns the Calc escapement style. */
    SvxEscapement       GetScEscapement() const;
    /** Returns the Calc strike-out style. */
    FontStrikeout       GetScStrikeout() const;

    /** Sets the Calc font height (in twips). */
    void                SetScHeight( sal_Int32 nTwips );
    /** Sets the Calc font family. */
    void                SetScFamily( FontFamily eScFamily );
    /** Sets the font text encoding. */
    void                SetFontEncoding( rtl_TextEncoding eFontEnc );
    /** Sets the Calc font posture. */
    void                SetScPosture( FontItalic eScPosture );
    /** Sets the Calc font weight. */
    void                SetScWeight( FontWeight eScWeight );
    /** Sets the Calc underline style. */
    void                SetScUnderline( FontLineStyle eScUnderl );
    /** Sets the Calc escapement style. */
    void                SetScEscapement( short nScEscapem );
    /** Sets the Calc strike-out style. */
    void                SetScStrikeout( FontStrikeout eScStrikeout );

// *** conversion of API constants *** ----------------------------------------

    /** Returns the API font height. */
    float               GetApiHeight() const;
    /** Returns the API font family. */
    sal_Int16           GetApiFamily() const;
    /** Returns the API font text encoding. */
    sal_Int16           GetApiFontEncoding() const;
    /** Returns the API font posture. */
    css::awt::FontSlant GetApiPosture() const;
    /** Returns the API font weight. */
    float               GetApiWeight() const;
    /** Returns the API font underline style. */
    sal_Int16           GetApiUnderline() const;
    /** Returns the API escapement style. */
    sal_Int16           GetApiEscapement() const;
    /** Returns the API font strike-out style. */
    sal_Int16           GetApiStrikeout() const;

    /** Sets the API font height. */
    void                SetApiHeight( float fPoint );
    /** Sets the API font family. */
    void                SetApiFamily( sal_Int16 nApiFamily );
    /** Sets the API font posture. */
    void                SetApiPosture( css::awt::FontSlant eApiPosture );
    /** Sets the API font weight. */
    void                SetApiWeight( float fApiWeight );
    /** Sets the API font underline style. */
    void                SetApiUnderline( sal_Int16 nApiUnderl );
    /** Sets the API escapement style. */
    void                SetApiEscapement( sal_Int16 nApiEscapem );
    /** Sets the API font strike-out style. */
    void                SetApiStrikeout( sal_Int16 nApiStrikeout );
};

bool operator==( const XclFontData& rLeft, const XclFontData& rRight );

/** Enumerates different types of Which-IDs for font items. */
enum class XclFontItemType
{
    Cell,          /// Use Calc Which-IDs (ATTR_*).
    Editeng,       /// Use edit engine Which-IDs (EE_CHAR_*).
    HeaderFooter   /// Use header/footer edit engine Which-IDs (EE_CHAR_*).
};

/** Enumerates different types for objects with font settings (using different property names). */
enum XclFontPropSetType
{
    EXC_FONTPROPSET_CHART,          /// All text objects in charts.
    EXC_FONTPROPSET_CONTROL         /// Text formatting in form controls.
};

/** Helper class for usage of property sets. */
class XclFontPropSetHelper
{
public:
    explicit            XclFontPropSetHelper();

    /** Reads all font properties from the passed property set. */
    void                ReadFontProperties( XclFontData& rFontData,
                            const ScfPropertySet& rPropSet, XclFontPropSetType eType,
                            sal_Int16 nScript = -1 );

    /** Writes all font properties to the passed property set, uses passed color as font color. */
    void                WriteFontProperties(
                            ScfPropertySet& rPropSet, XclFontPropSetType eType,
                            const XclFontData& rFontData,
                            bool bHasWstrn, bool bHasAsian, bool bHasCmplx,
                            const Color* pFontColor );

private:
    /** Returns a chart property set helper according to the passed script type. */
    ScfPropSetHelper&   GetChartHelper( sal_Int16 nScript );

private:
    ScfPropSetHelper    maHlpChCommon;      /// Chart properties for all scripts.
    ScfPropSetHelper    maHlpChWstrn;       /// Chart properties for Western script.
    ScfPropSetHelper    maHlpChAsian;       /// Chart properties for Asian script.
    ScfPropSetHelper    maHlpChCmplx;       /// Chart properties for Complex script.
    ScfPropSetHelper    maHlpChWstrnNoName; /// Chart properties for Western script, no font name.
    ScfPropSetHelper    maHlpChAsianNoName; /// Chart properties for Asian script, no font name.
    ScfPropSetHelper    maHlpChCmplxNoName; /// Chart properties for Complex script, no font name.
    ScfPropSetHelper    maHlpChEscapement;  /// Chart properties for font escapement.
    ScfPropSetHelper    maHlpControl;       /// Properties for form controls.
};

// Number formats =============================================================

struct XclNumFmt
{
    OUString            maFormat;       /// Format string, may be empty (meOffset used then).
    NfIndexTableOffset  meOffset;       /// SvNumberFormatter format index, if maFormat is empty.
    LanguageType        meLanguage;     /// Language type to be set with the number format.
};

class XclNumFmtBuffer
{
public:
    explicit            XclNumFmtBuffer( const XclRoot& rRoot );

    /** Returns the core index of the current standard number format. */
    sal_uInt32          GetStdScNumFmt() const { return mnStdScNumFmt; }

protected:
    typedef ::std::map< sal_uInt16, XclNumFmt > XclNumFmtMap;

    /** Clears all buffered data, used to set up for a new sheet. */
    void                InitializeImport();

    /** Returns the current number format map. */
    const XclNumFmtMap& GetFormatMap() const { return maFmtMap; }

    /** Inserts a new number format for the specified Excel format index. */
    void                InsertFormat( sal_uInt16 nXclNumFmt, const OUString& rFormat );

private:
    /** Inserts built-in number formats for the current system language. */
    void                InsertBuiltinFormats();

    XclNumFmtMap        maFmtMap;       /// Map containing all default and user-defined formats.
    const LanguageType  meSysLang;      /// Current system language.
    const sal_uInt32    mnStdScNumFmt;  /// Calc format key for standard number format.
};

// Cell formatting data (XF) ==================================================

/** Contains all cell protection attributes. */
struct XclCellProt
{
    bool                mbLocked;       /// true = Locked against editing.
    bool                mbHidden;       /// true = Formula is hidden.

    explicit            XclCellProt();
};

bool operator==( const XclCellProt& rLeft, const XclCellProt& rRight );

/** Contains all cell alignment attributes. */
struct XclCellAlign
{
    sal_uInt8           mnHorAlign;     /// Horizontal alignment.
    sal_uInt8           mnVerAlign;     /// Vertical alignment.
    sal_uInt8           mnOrient;       /// Text orientation.
    sal_uInt8           mnTextDir;      /// CTL text direction.
    sal_uInt8           mnRotation;     /// Text rotation angle.
    sal_uInt8           mnIndent;       /// Indentation.
    bool                mbLineBreak;    /// true = Multi-line text.
    bool                mbShrink;       /// true = Shrink to fit cell size.

    explicit            XclCellAlign();

    /** Returns the Calc horizontal alignment. */
    SvxCellHorJustify   GetScHorAlign() const;
    /** Returns horizontal justification method as Calc's attribute. */
    SvxCellJustifyMethod GetScHorJustifyMethod() const;
    /** Returns the Calc vertical alignment. */
    SvxCellVerJustify   GetScVerAlign() const;
    /** Returns vertical justification method as Calc's attribute. */
    SvxCellJustifyMethod GetScVerJustifyMethod() const;
    /** Returns the Calc frame direction. */
    SvxFrameDirection   GetScFrameDir() const;

    /** Sets the Calc horizontal alignment. */
    void                SetScHorAlign( SvxCellHorJustify eHorJust );
    /** Sets the Calc vertical alignment. */
    void                SetScVerAlign( SvxCellVerJustify eVerJust );
    /** Sets the Calc frame direction. */
    void                SetScFrameDir( SvxFrameDirection eFrameDir );
};

bool operator==( const XclCellAlign& rLeft, const XclCellAlign& rRight );

/** Contains color and line style for each cell border line. */
struct XclCellBorder
{
    sal_uInt16          mnLeftColor;    /// Palette index for left line.
    sal_uInt16          mnRightColor;   /// Palette index for right line.
    sal_uInt16          mnTopColor;     /// Palette index for top line.
    sal_uInt16          mnBottomColor;  /// Palette index for bottom line.
    sal_uInt16          mnDiagColor;    /// Palette index for diagonal line(s).
    sal_uInt8           mnLeftLine;     /// Style of left line.
    sal_uInt8           mnRightLine;    /// Style of right line.
    sal_uInt8           mnTopLine;      /// Style of top line.
    sal_uInt8           mnBottomLine;   /// Style of bottom line.
    sal_uInt8           mnDiagLine;     /// Style of diagonal line(s).
    bool                mbDiagTLtoBR;   /// true = Top-left to bottom-right on.
    bool                mbDiagBLtoTR;   /// true = Bottom-left to top-right on.

    explicit            XclCellBorder();
};

bool operator==( const XclCellBorder& rLeft, const XclCellBorder& rRight );

/** Contains background colors and pattern for a cell. */
struct XclCellArea
{
    sal_uInt16          mnForeColor;    /// Palette index to foreground color.
    sal_uInt16          mnBackColor;    /// Palette index to background color.
    sal_uInt8           mnPattern;      /// Fill pattern.

    explicit            XclCellArea();

    /** Returns true, if the area represents transparent state. */
    bool                IsTransparent() const;
};

bool operator==( const XclCellArea& rLeft, const XclCellArea& rRight );

/** Contains base members for XF record import/export.
    @descr  In detail this class stores the XF type (cell/style), the index to the
    parent style XF and all "attribute used" flags, which reflect the state of
    specific attribute groups (true = user has changed the attributes). */
class XclXFBase
{
public:
    explicit            XclXFBase( bool bCellXF );
    virtual             ~XclXFBase();

    XclXFBase(XclXFBase const &) = default;
    XclXFBase(XclXFBase &&) = default;
    XclXFBase & operator =(XclXFBase const &) = default;
    XclXFBase & operator =(XclXFBase &&) = default;

    /** Sets all "attribute used" flags to the passed state. */
    void                SetAllUsedFlags( bool bUsed );
    /** Returns true, if any "attribute used" flags are ste in this XF. */
    bool                HasUsedFlags() const;

    /** Returns true, if this is a hard cell format. */
    bool         IsCellXF() const    { return mbCellXF; }
    /** Returns true, if this is a cell style. */
    bool         IsStyleXF() const   { return !IsCellXF(); }

protected:
    /** Returns true, if this object is equal to the passed. */
    bool                Equals( const XclXFBase& rCmp ) const;

protected:
    sal_uInt16          mnParent;           /// Index to parent style XF.
    bool                mbCellXF;           /// true = cell XF, false = style XF.
    bool                mbProtUsed;         /// true = cell protection used.
    bool                mbFontUsed;         /// true = font index used.
    bool                mbFmtUsed;          /// true = number format used.
    bool                mbAlignUsed;        /// true = alignment used.
    bool                mbBorderUsed;       /// true = border data used.
    bool                mbAreaUsed;         /// true = area data used.
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
