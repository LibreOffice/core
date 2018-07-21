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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_STYLESBUFFER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_STYLESBUFFER_HXX

#include <memory>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/util/CellProtection.hpp>
#include <oox/drawingml/color.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/helper/refmap.hxx>
#include <oox/helper/refvector.hxx>
#include "numberformatsbuffer.hxx"
#include <patattr.hxx>
#include <stlsheet.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/frmdir.hxx>
#include <attarray.hxx>
#include <vector>

namespace oox { class PropertySet;
                class PropertyMap; }

namespace oox {
namespace xls {

const sal_Int32 OOX_COLOR_WINDOWTEXT3       = 24;       /// System window text color (BIFF3-BIFF4).
const sal_Int32 OOX_COLOR_WINDOWBACK3       = 25;       /// System window background color (BIFF3-BIFF4).
const sal_Int32 OOX_COLOR_WINDOWTEXT        = 64;       /// System window text color (BIFF5+).
const sal_Int32 OOX_COLOR_WINDOWBACK        = 65;       /// System window background color (BIFF5+).
const sal_Int32 OOX_COLOR_BUTTONBACK        = 67;       /// System button background color (face color).
const sal_Int32 OOX_COLOR_CHWINDOWTEXT      = 77;       /// System window text color (BIFF8 charts).
const sal_Int32 OOX_COLOR_CHWINDOWBACK      = 78;       /// System window background color (BIFF8 charts).
const sal_Int32 OOX_COLOR_CHBORDERAUTO      = 79;       /// Automatic frame border (BIFF8 charts).
const sal_Int32 OOX_COLOR_NOTEBACK          = 80;       /// Note background color.
const sal_Int32 OOX_COLOR_NOTETEXT          = 81;       /// Note text color.
const sal_Int32 OOX_COLOR_FONTAUTO          = 0x7FFF;   /// Font auto color (system window text color).

const sal_Int16 API_LINE_NONE               = 0;
const sal_Int16 API_LINE_HAIR               = 1;
const sal_Int16 API_LINE_THIN               = 15;
const sal_Int16 API_LINE_MEDIUM             = 35;
const sal_Int16 API_LINE_THICK              = 50;

const sal_Int16 API_ESCAPE_NONE             = 0;        /// No escapement.
const sal_Int16 API_ESCAPE_SUPERSCRIPT      = 101;      /// Superscript: raise characters automatically (magic value 101).
const sal_Int16 API_ESCAPE_SUBSCRIPT        = -101;     /// Subscript: lower characters automatically (magic value -101).

const sal_Int8 API_ESCAPEHEIGHT_NONE        = 100;      /// Relative character height if not escaped.
const sal_Int8 API_ESCAPEHEIGHT_DEFAULT     = 58;       /// Relative character height if escaped.

/** Special implementation of the GraphicHelper for Excel palette and scheme
    colors.
 */
class ExcelGraphicHelper : public GraphicHelper, public WorkbookHelper
{
public:
    explicit            ExcelGraphicHelper( const WorkbookHelper& rHelper );

    /** Derived classes may implement to resolve a scheme color from the passed XML token identifier. */
    virtual ::Color     getSchemeColor( sal_Int32 nToken ) const override;
    /** Derived classes may implement to resolve a palette index to an RGB color. */
    virtual ::Color     getPaletteColor( sal_Int32 nPaletteIdx ) const override;
};

class Color : public ::oox::drawingml::Color
{
public:
    /** Sets the color to automatic. */
    void                setAuto();
    /** Sets the color to the passed RGB value. */
    void                setRgb( ::Color nRgbValue, double fTint = 0.0 );
    /** Sets the color to the passed theme index. */
    void                setTheme( sal_Int32 nThemeIdx, double fTint = 0.0 );
    /** Sets the color to the passed palette index. */
    void                setIndexed( sal_Int32 nPaletteIdx, double fTint = 0.0 );

    /** Imports the color from the passed attribute list. */
    void                importColor( const AttributeList& rAttribs );

    /** Imports a 64-bit color from the passed binary stream. */
    void                importColor( SequenceInputStream& rStrm );
    /** Imports a 32-bit palette color identifier from the passed BIFF12 stream. */
    void                importColorId( SequenceInputStream& rStrm );

    /** Returns true, if the color is set to automatic. */
    bool         isAuto() const { return isPlaceHolder(); }
};

SequenceInputStream& operator>>( SequenceInputStream& rStrm, Color& orColor );

/** Stores all colors of the color palette. */
class ColorPalette : public WorkbookHelper
{
public:
    /** Constructs the color palette with predefined color values. */
    explicit            ColorPalette( const WorkbookHelper& rHelper );

    /** Appends a new color from the passed attributes. */
    void                importPaletteColor( const AttributeList& rAttribs );
    /** Appends a new color from the passed RGBCOLOR record. */
    void                importPaletteColor( SequenceInputStream& rStrm );

    /** Returns the RGB value of the color with the passed index. */
    ::Color             getColor( sal_Int32 nPaletteIdx ) const;

private:
    /** Appends the passed color. */
    void                appendColor( ::Color nRGBValue );

private:
    ::std::vector< ::Color > maColors;    /// List of RGB values.
    size_t              mnAppendIndex;      /// Index to append a new color.
};

/** Contains all XML font attributes, e.g. from a font or rPr element. */
struct FontModel
{
    OUString     maName;             /// Font name.
    Color               maColor;            /// Font color.
    sal_Int32           mnScheme;           /// Major/minor scheme font.
    sal_Int32           mnFamily;           /// Font family.
    sal_Int32           mnCharSet;          /// Windows font character set.
    double              mfHeight;           /// Font height in points.
    sal_Int32           mnUnderline;        /// Underline style.
    sal_Int32           mnEscapement;       /// Escapement style.
    bool                mbBold;             /// True = bold characters.
    bool                mbItalic;           /// True = italic characters.
    bool                mbStrikeout;        /// True = Strike out characters.
    bool                mbOutline;          /// True = outlined characters.
    bool                mbShadow;           /// True = shadowed chgaracters.

    explicit            FontModel();

    void                setBiff12Scheme( sal_uInt8 nScheme );
    void                setBiffHeight( sal_uInt16 nHeight );
    void                setBiffWeight( sal_uInt16 nWeight );
    void                setBiffUnderline( sal_uInt16 nUnderline );
    void                setBiffEscapement( sal_uInt16 nEscapement );
};

/** Contains used flags for all API font attributes. */
struct ApiFontUsedFlags
{
    bool                mbNameUsed;         /// True = font name/family/char set are used.
    bool                mbColorUsed;        /// True = font color is used.
    bool                mbSchemeUsed;       /// True = font scheme is used.
    bool                mbHeightUsed;       /// True = font height is used.
    bool                mbUnderlineUsed;    /// True = underline style is used.
    bool                mbEscapementUsed;   /// True = escapement style is used.
    bool                mbWeightUsed;       /// True = font weight (boldness) is used.
    bool                mbPostureUsed;      /// True = font posture (italic) is used.
    bool                mbStrikeoutUsed;    /// True = strike out style is used.
    bool                mbOutlineUsed;      /// True = outline style is used.
    bool                mbShadowUsed;       /// True = shadow style is used.

    explicit            ApiFontUsedFlags( bool bAllUsed );
};

/** Contains API font name, family, and charset for a script type. */
struct ApiScriptFontName
{
    OUString     maName;             /// Font name.
    sal_Int16           mnFamily;           /// Font family.
    sal_Int16           mnTextEnc;          /// Font text encoding.

    explicit            ApiScriptFontName();
};

/** Contains all API font attributes. */
struct ApiFontData
{
    ApiScriptFontName        maLatinFont;        /// Font name for latin scripts.
    ApiScriptFontName        maAsianFont;        /// Font name for east-asian scripts.
    ApiScriptFontName        maCmplxFont;        /// Font name for complex scripts.
    css::awt::FontDescriptor maDesc;             /// Font descriptor (height in twips, weight in %).
    ::Color                  mnColor;            /// Font color.
    sal_Int16                mnEscapement;       /// Escapement style.
    sal_Int8                 mnEscapeHeight;     /// Escapement font height.
    bool                     mbOutline;          /// True = outlined characters.
    bool                     mbShadow;           /// True = shadowed chgaracters.

    explicit            ApiFontData();
};

class Font : public WorkbookHelper
{
public:
    explicit            Font( const WorkbookHelper& rHelper, bool bDxf );
    explicit            Font( const WorkbookHelper& rHelper, const FontModel& rModel );

    /** Sets font formatting attributes for the passed element. */
    void                importAttribs( sal_Int32 nElement, const AttributeList& rAttribs );

    /** Imports the FONT record from the passed stream. */
    void                importFont( SequenceInputStream& rStrm );
    /** Imports the font name from a DXF record. */
    void                importDxfName( SequenceInputStream& rStrm );
    /** Imports the font color from a DXF record. */
    void                importDxfColor( SequenceInputStream& rStrm );
    /** Imports the font scheme from a DXF record. */
    void                importDxfScheme( SequenceInputStream& rStrm );
    /** Imports the font height from a DXF record. */
    void                importDxfHeight( SequenceInputStream& rStrm );
    /** Imports the font weight from a DXF record. */
    void                importDxfWeight( SequenceInputStream& rStrm );
    /** Imports the font underline style from a DXF record. */
    void                importDxfUnderline( SequenceInputStream& rStrm );
    /** Imports the font escapement style from a DXF record. */
    void                importDxfEscapement( SequenceInputStream& rStrm );
    /** Imports a font style flag from a DXF record. */
    void                importDxfFlag( sal_Int32 nElement, SequenceInputStream& rStrm );

    /** Returns the font model structure. This function can be called before
        finalizeImport() has been called. */
    const FontModel& getModel() const { return maModel; }

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    /** Returns an API font descriptor with own font information. */
    const css::awt::FontDescriptor& getFontDescriptor() const { return maApiData.maDesc;}
    /** Returns true, if the font requires rich text formatting in Calc.
        @descr  Example: Font escapement is a cell attribute in Excel, but Calc
        needs an rich text cell for this attribute. */
    bool                needsRichTextFormat() const;

    void                fillToItemSet( SfxItemSet& rItemSet, bool bEditEngineText, bool bSkipPoolDefs = false ) const;
    /** Writes all font attributes to the passed property map. */
    void                writeToPropertyMap(
                            PropertyMap& rPropMap ) const;
    /** Writes all font attributes to the passed property set. */
    void                writeToPropertySet(
                            PropertySet& rPropSet ) const;

private:
    FontModel           maModel;
    ApiFontData         maApiData;
    ApiFontUsedFlags    maUsedFlags;
    bool                mbDxf;
};

typedef std::shared_ptr< Font > FontRef;

/** Contains all XML cell alignment attributes, e.g. from an alignment element. */
struct AlignmentModel
{
    sal_Int32           mnHorAlign;         /// Horizontal alignment.
    sal_Int32           mnVerAlign;         /// Vertical alignment.
    sal_Int32           mnTextDir;          /// CTL text direction.
    sal_Int32           mnRotation;         /// Text rotation angle.
    sal_Int32           mnIndent;           /// Indentation.
    bool                mbWrapText;         /// True = multi-line text.
    bool                mbShrink;           /// True = shrink to fit cell size.
    bool                mbJustLastLine;     /// True = justify last line in block text.

    explicit            AlignmentModel();

    /** Sets horizontal alignment from the passed BIFF data. */
    void                setBiffHorAlign( sal_uInt8 nHorAlign );
    /** Sets vertical alignment from the passed BIFF data. */
    void                setBiffVerAlign( sal_uInt8 nVerAlign );
};

/** Contains all API cell alignment attributes. */
struct ApiAlignmentData
{
    css::table::CellHoriJustify  meHorJustify;       /// Horizontal alignment.
    sal_Int32                    mnHorJustifyMethod;
    sal_Int32                    mnVerJustify;       /// Vertical alignment.
    sal_Int32                    mnVerJustifyMethod;
    css::table::CellOrientation  meOrientation;      /// Normal or stacked text.
    sal_Int32                    mnRotation;         /// Text rotation angle.
    sal_Int16                    mnWritingMode;      /// CTL text direction.
    sal_Int16                    mnIndent;           /// Indentation.
    bool                         mbWrapText;         /// True = multi-line text.
    bool                         mbShrink;           /// True = shrink to fit cell size.

    explicit            ApiAlignmentData();
};

bool operator==( const ApiAlignmentData& rLeft, const ApiAlignmentData& rRight );

class Alignment : public WorkbookHelper
{
public:
    explicit            Alignment( const WorkbookHelper& rHelper );

    /** Sets all attributes from the alignment element. */
    void                importAlignment( const AttributeList& rAttribs );

    /** Sets the alignment attributes from the passed BIFF12 XF record data. */
    void                setBiff12Data( sal_uInt32 nFlags );

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    /** Returns the alignment model structure. */
    const AlignmentModel& getModel() const { return maModel; }
    /** Returns the converted API alignment data struct. */
    const ApiAlignmentData& getApiData() const { return maApiData; }

    void                fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs = false ) const;

private:
    ::SvxCellHorJustify GetScHorAlign() const;
    ::SvxCellVerJustify GetScVerAlign() const;
    ::SvxFrameDirection GetScFrameDir() const;
    AlignmentModel      maModel;            /// Alignment model data.
    ApiAlignmentData    maApiData;          /// Alignment data converted to API constants.
};

/** Contains all XML cell protection attributes, e.g. from a protection element. */
struct ProtectionModel
{
    bool                mbLocked;           /// True = locked against editing.
    bool                mbHidden;           /// True = formula is hidden.

    explicit            ProtectionModel();
};

/** Contains all API cell protection attributes. */
struct ApiProtectionData
{
    typedef css::util::CellProtection ApiCellProtection;

    ApiCellProtection   maCellProt;

    explicit            ApiProtectionData();
};

bool operator==( const ApiProtectionData& rLeft, const ApiProtectionData& rRight );

class Protection : public WorkbookHelper
{
public:
    explicit            Protection( const WorkbookHelper& rHelper );

    /** Sets all attributes from the protection element. */
    void                importProtection( const AttributeList& rAttribs );

    /** Sets the protection attributes from the passed BIFF12 XF record data. */
    void                setBiff12Data( sal_uInt32 nFlags );

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    /** Returns the converted API protection data struct. */
    const ApiProtectionData& getApiData() const { return maApiData; }

    void                fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs = false ) const;
private:
    ProtectionModel     maModel;            /// Protection model data.
    ApiProtectionData   maApiData;          /// Protection data converted to API constants.
};

/** Contains XML attributes of a single border line. */
struct BorderLineModel
{
    Color               maColor;            /// Borderline color.
    sal_Int32           mnStyle;            /// Border line style.
    bool                mbUsed;             /// True = line format used.

    explicit            BorderLineModel( bool bDxf );

    /** Sets the passed BIFF line style. */
    void                setBiffStyle( sal_Int32 nLineStyle );
};

/** Contains XML attributes of a complete cell border. */
struct BorderModel
{
    BorderLineModel     maLeft;             /// Left line format.
    BorderLineModel     maRight;            /// Right line format.
    BorderLineModel     maTop;              /// Top line format.
    BorderLineModel     maBottom;           /// Bottom line format.
    BorderLineModel     maDiagonal;         /// Diagonal line format.
    bool                mbDiagTLtoBR;       /// True = top-left to bottom-right on.
    bool                mbDiagBLtoTR;       /// True = bottom-left to top-right on.

    explicit            BorderModel( bool bDxf );
};

/** Contains API attributes of a complete cell border. */
struct ApiBorderData
{
    typedef css::table::BorderLine2     ApiBorderLine;

    ApiBorderLine       maLeft;             /// Left line format
    ApiBorderLine       maRight;            /// Right line format
    ApiBorderLine       maTop;              /// Top line format
    ApiBorderLine       maBottom;           /// Bottom line format
    ApiBorderLine       maTLtoBR;           /// Diagonal top-left to bottom-right line format.
    ApiBorderLine       maBLtoTR;           /// Diagonal bottom-left to top-right line format.
    bool                mbBorderUsed;       /// True = left/right/top/bottom line format used.
    bool                mbDiagUsed;         /// True = diagonal line format used.

    explicit            ApiBorderData();

    /** Returns true, if any of the outer border lines is visible. */
    bool                hasAnyOuterBorder() const;
};

class Border : public WorkbookHelper
{
public:
    explicit            Border( const WorkbookHelper& rHelper, bool bDxf );

    /** Sets global border attributes from the border element. */
    void                importBorder( const AttributeList& rAttribs );
    /** Sets border attributes for the border line with the passed element identifier. */
    void                importStyle( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Sets color attributes for the border line with the passed element identifier. */
    void                importColor( sal_Int32 nElement, const AttributeList& rAttribs );

    /** Imports the BORDER record from the passed stream. */
    void                importBorder( SequenceInputStream& rStrm );
    /** Imports a border from a DXF record from the passed stream. */
    void                importDxfBorder( sal_Int32 nElement, SequenceInputStream& rStrm );

    /** Final processing after import of all style settings. */
    void                finalizeImport( bool bRTL );

    /** Returns the converted API border data struct. */
    const ApiBorderData& getApiData() const { return maApiData; }

    void fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs = false ) const;

private:
    /** Returns the border line struct specified by the passed XML token identifier. */
    BorderLineModel*    getBorderLine( sal_Int32 nElement );

    /** Converts border line data to an API struct, returns true, if the line is marked as used. */
    bool                convertBorderLine(
                            css::table::BorderLine2& rBorderLine,
                            const BorderLineModel& rModel );

private:
    BorderModel         maModel;
    ApiBorderData       maApiData;
    bool                mbDxf;
};

typedef std::shared_ptr< Border > BorderRef;

/** Contains XML pattern fill attributes from the patternFill element. */
struct PatternFillModel
{
    Color               maPatternColor;     /// Pattern foreground color.
    Color               maFillColor;        /// Background fill color.
    sal_Int32           mnPattern;          /// Pattern identifier (e.g. solid).
    bool                mbPattColorUsed;    /// True = pattern foreground color used.
    bool                mbFillColorUsed;    /// True = background fill color used.
    bool                mbPatternUsed;      /// True = pattern used.

    explicit            PatternFillModel( bool bDxf );

    /** Sets the passed BIFF pattern identifier. */
    void                setBiffPattern( sal_Int32 nPattern );
};

/** Contains XML gradient fill attributes from the gradientFill element. */
struct GradientFillModel
{
    typedef ::std::map< double, Color > ColorMap;

    sal_Int32           mnType;             /// Gradient type, linear or path.
    double              mfAngle;            /// Rotation angle for type linear.
    double              mfLeft;             /// Left convergence for type path.
    double              mfRight;            /// Right convergence for type path.
    double              mfTop;              /// Top convergence for type path.
    double              mfBottom;           /// Bottom convergence for type path.
    ColorMap            maColors;           /// Gradient colors.

    explicit            GradientFillModel();

    /** Reads BIFF12 gradient settings from a FILL or DXF record. */
    void                readGradient( SequenceInputStream& rStrm );
    /** Reads BIFF12 gradient stop settings from a FILL or DXF record. */
    void                readGradientStop( SequenceInputStream& rStrm, bool bDxf );
};

/** Contains API fill attributes. */
struct ApiSolidFillData
{
    ::Color             mnColor;            /// Fill color.
    bool                mbTransparent;      /// True = transparent area.
    bool                mbUsed;             /// True = fill data is valid.

    explicit            ApiSolidFillData();
};

/** Contains cell fill attributes, either a pattern fill or a gradient fill. */
class Fill : public WorkbookHelper
{
public:
    explicit            Fill( const WorkbookHelper& rHelper, bool bDxf );

    /** Sets attributes of a patternFill element. */
    void                importPatternFill( const AttributeList& rAttribs );
    /** Sets the pattern color from the fgColor element. */
    void                importFgColor( const AttributeList& rAttribs );
    /** Sets the background color from the bgColor element. */
    void                importBgColor( const AttributeList& rAttribs );
    /** Sets attributes of a gradientFill element. */
    void                importGradientFill( const AttributeList& rAttribs );
    /** Sets a color from the color element in a gradient fill. */
    void                importColor( const AttributeList& rAttribs, double fPosition );

    /** Imports the FILL record from the passed stream. */
    void                importFill( SequenceInputStream& rStrm );
    /** Imports the fill pattern from a DXF record. */
    void                importDxfPattern( SequenceInputStream& rStrm );
    /** Imports the pattern color from a DXF record. */
    void                importDxfFgColor( SequenceInputStream& rStrm );
    /** Imports the background color from a DXF record. */
    void                importDxfBgColor( SequenceInputStream& rStrm );
    /** Imports gradient settings from a DXF record. */
    void                importDxfGradient( SequenceInputStream& rStrm );
    /** Imports gradient stop settings from a DXF record. */
    void                importDxfStop( SequenceInputStream& rStrm );

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    void                fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs = false ) const;

private:
    typedef std::shared_ptr< PatternFillModel >   PatternModelRef;
    typedef std::shared_ptr< GradientFillModel >  GradientModelRef;

    PatternModelRef     mxPatternModel;
    GradientModelRef    mxGradientModel;
    ApiSolidFillData    maApiData;
    bool                mbDxf;
};

typedef std::shared_ptr< Fill > FillRef;

/** Contains all data for a cell format or cell style. */
struct XfModel
{
    sal_Int32           mnStyleXfId;        /// Index to parent style XF.
    sal_Int32           mnFontId;           /// Index to font data list.
    sal_Int32           mnNumFmtId;         /// Index to number format list.
    sal_Int32           mnBorderId;         /// Index to list of cell borders.
    sal_Int32           mnFillId;           /// Index to list of cell areas.
    bool                mbCellXf;           /// True = cell XF, false = style XF.
    bool                mbFontUsed;         /// True = font index used.
    bool                mbNumFmtUsed;       /// True = number format used.
    bool                mbAlignUsed;        /// True = alignment used.
    bool                mbProtUsed;         /// True = cell protection used.
    bool                mbBorderUsed;       /// True = border data used.
    bool                mbAreaUsed;         /// True = area data used.

    explicit            XfModel();
};

bool operator==( const XfModel& rXfModel1,  const XfModel& rXfModel2 );

/** Represents a cell format or a cell style (called XF, extended format).

    This class stores the type (cell/style), the index to the parent style (if
    it is a cell format) and all "attribute used" flags, which reflect the
    state of specific attribute groups (true = user has changed the attributes)
    and all formatting data.
 */
class Xf : public WorkbookHelper
{
    friend bool operator==( const Xf& rXf1,  const Xf& rXf2 );
public:
    struct AttrList
    {
        std::vector<ScAttrEntry> maAttrs;
        bool mbLatinNumFmtOnly;
        const ScPatternAttr* mpDefPattern;

        AttrList(const ScPatternAttr* pDefPatternAttr);
    };

    explicit            Xf( const WorkbookHelper& rHelper );

    /** Sets all attributes from the xf element. */
    void                importXf( const AttributeList& rAttribs, bool bCellXf );
    /** Sets all attributes from the alignment element. */
    void                importAlignment( const AttributeList& rAttribs );
    /** Sets all attributes from the protection element. */
    void                importProtection( const AttributeList& rAttribs );

    /** Imports the XF record from the passed stream. */
    void                importXf( SequenceInputStream& rStrm, bool bCellXf );

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    /** Returns true, if the XF is a cell XF, and false, if it is a style XF. */
    bool         isCellXf() const { return maModel.mbCellXf; }

    /** Returns the referred font object. */
    FontRef             getFont() const;
    /** Returns the alignment data of this style. */
    const Alignment& getAlignment() const { return maAlignment; }

    void applyPatternToAttrList(
        AttrList& rAttrs, SCROW nRow1, SCROW nRow2, sal_Int32 nForceScNumFmt );

    void writeToDoc( ScDocumentImport& rDoc, const ScRange& rRange );

    const ::ScPatternAttr& createPattern( bool bSkipPoolDefs = false );

private:
    typedef ::std::unique_ptr< ::ScPatternAttr > ScPatternAttrPtr;

    ScPatternAttrPtr    mpPattern;          /// Calc item set.
    sal_uInt32          mnScNumFmt;         /// Calc number format.

    XfModel             maModel;            /// Cell XF or style XF model data.
    Alignment           maAlignment;        /// Cell alignment data.
    Protection          maProtection;       /// Cell protection data.
    sal_Int32           meRotationRef;      /// Rotation reference dependent on border.
    ::ScStyleSheet*       mpStyleSheet;       /// Calc cell style sheet.
};

bool operator==( const Xf& rXf1,  const Xf& rXf2 );

typedef std::shared_ptr< Xf > XfRef;

class Dxf : public WorkbookHelper
{
public:
    explicit            Dxf( const WorkbookHelper& rHelper );

    /** Creates a new empty font object. */
    FontRef const &     createFont( bool bAlwaysNew = true );
    /** Creates a new empty border object. */
    BorderRef const &   createBorder( bool bAlwaysNew = true );
    /** Creates a new empty fill object. */
    FillRef const &     createFill( bool bAlwaysNew = true );

    /** Inserts a new number format code. */
    void                importNumFmt( const AttributeList& rAttribs );

    /** Imports the DXF record from the passed stream. */
    void                importDxf( SequenceInputStream& rStrm );

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    void fillToItemSet( SfxItemSet& rSet ) const;

private:
    FontRef             mxFont;             /// Font data.
    NumberFormatRef     mxNumFmt;           /// Number format data.
    std::shared_ptr< Alignment >
                        mxAlignment;        /// Alignment data.
    std::shared_ptr< Protection >
                        mxProtection;       /// Protection data.
    BorderRef           mxBorder;           /// Border data.
    FillRef             mxFill;             /// Fill data.
};

typedef std::shared_ptr< Dxf > DxfRef;

/** Contains attributes of a cell style, e.g. from the cellStyle element. */
struct CellStyleModel
{
    OUString     maName;             /// Cell style name.
    sal_Int32           mnXfId;             /// Formatting for this cell style.
    sal_Int32           mnBuiltinId;        /// Identifier for builtin styles.
    sal_Int32           mnLevel;            /// Level for builtin column/row styles.
    bool                mbBuiltin;          /// True = builtin style.
    bool                mbCustom;           /// True = customized builtin style.
    bool                mbHidden;           /// True = style not visible in GUI.

    explicit            CellStyleModel();

    /** Returns true, if this style is a builtin style. */
    bool                isBuiltin() const;
    /** Returns true, if this style represents the default document cell style. */
    bool                isDefaultStyle() const;
};

class CellStyle : public WorkbookHelper
{
public:
    explicit            CellStyle( const WorkbookHelper& rHelper );

    /** Imports passed attributes from the cellStyle element. */
    void                importCellStyle( const AttributeList& rAttribs );
    /** Imports style settings from a CELLSTYLE record. */
    void                importCellStyle( SequenceInputStream& rStrm );

    /** Creates the style sheet in the document described by this cell style object. */
    void                createCellStyle();
    /** Stores the passed final style name and creates the cell style, if it is
        user-defined or modified built-in. */
    void                finalizeImport( const OUString& rFinalName );

    /** Returns the cell style model structure. */
    const CellStyleModel& getModel() const { return maModel; }
    /** Returns the final style name used in the document. */
    const OUString& getFinalStyleName() const { return maFinalName; }
    ::ScStyleSheet* getStyleSheet() { return mpStyleSheet; }
private:
    CellStyleModel      maModel;
    OUString     maFinalName;        /// Final style name used in API.
    bool                mbCreated;          /// True = style sheet created.
    ::ScStyleSheet*     mpStyleSheet;       /// Calc cell style sheet.

};

typedef std::shared_ptr< CellStyle > CellStyleRef;

class CellStyleBuffer : public WorkbookHelper
{
public:
    explicit            CellStyleBuffer( const WorkbookHelper& rHelper );

    /** Appends and returns a new named cell style object. */
    CellStyleRef        importCellStyle( const AttributeList& rAttribs );
    /** Imports the CELLSTYLE record from the passed stream. */
    CellStyleRef        importCellStyle( SequenceInputStream& rStrm );

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    /** Returns the XF identifier associated to the default cell style. */
    sal_Int32           getDefaultXfId() const;
    /** Returns the default style sheet for unused cells. */
    OUString     getDefaultStyleName() const;
    /** Creates the style sheet described by the style XF with the passed identifier. */
    OUString     createCellStyle( sal_Int32 nXfId ) const;
    ::ScStyleSheet*     getCellStyleSheet( sal_Int32 nXfId ) const;

private:
    /** Inserts the passed cell style object into the internal maps. */
    void                insertCellStyle( CellStyleRef const & xCellStyle );
    /** Creates the style sheet described by the passed cell style object. */
    static OUString     createCellStyle( const CellStyleRef& rxCellStyle );
    static ::ScStyleSheet* getCellStyleSheet( const CellStyleRef& rxCellStyle );

private:
    typedef RefVector< CellStyle >          CellStyleVector;
    typedef RefMap< sal_Int32, CellStyle >  CellStyleXfIdMap;

    CellStyleVector     maBuiltinStyles;    /// All built-in cell styles.
    CellStyleVector     maUserStyles;       /// All user defined cell styles.
    CellStyleXfIdMap    maStylesByXf;       /// All cell styles, mapped by XF identifier.
    CellStyleRef        mxDefStyle;         /// Default cell style.
};

struct AutoFormatModel
{
    sal_Int32           mnAutoFormatId;     /// Index of predefined autoformatting.
    bool                mbApplyNumFmt;      /// True = apply number format from autoformatting.
    bool                mbApplyFont;        /// True = apply font from autoformatting.
    bool                mbApplyAlignment;   /// True = apply alignment from autoformatting.
    bool                mbApplyBorder;      /// True = apply border from autoformatting.
    bool                mbApplyFill;        /// True = apply fill from autoformatting.
    bool                mbApplyProtection;  /// True = apply protection from autoformatting.

    explicit            AutoFormatModel();
};

class StylesBuffer : public WorkbookHelper
{
public:
    explicit            StylesBuffer( const WorkbookHelper& rHelper );

    /** Creates a new empty font object. */
    FontRef             createFont();
    /** Creates a number format. */
    NumberFormatRef     createNumFmt( sal_Int32 nNumFmtId, const OUString& rFmtCode );
    sal_Int32           nextFreeNumFmtId();
    /** Creates a new empty border object. */
    BorderRef           createBorder();
    /** Creates a new empty fill object. */
    FillRef             createFill();
    /** Creates a new empty cell formatting object. */
    XfRef               createCellXf();
    /** Creates a new empty style formatting object. */
    XfRef               createStyleXf();
    /** Creates a new empty differential formatting object. */
    DxfRef              createDxf();

    /** Appends a new color to the color palette. */
    void                importPaletteColor( const AttributeList& rAttribs );
    /** Inserts a new number format code. */
    NumberFormatRef     importNumFmt( const AttributeList& rAttribs );
    /** Appends and returns a new named cell style object. */
    CellStyleRef        importCellStyle( const AttributeList& rAttribs );

    /** Appends a new color to the color palette. */
    void                importPaletteColor( SequenceInputStream& rStrm );
    /** Imports the NUMFMT record from the passed stream. */
    void                importNumFmt( SequenceInputStream& rStrm );
    /** Imports the CELLSTYLE record from the passed stream. */
    void                importCellStyle( SequenceInputStream& rStrm );

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    /** Returns the palette color with the specified index. */
    ::Color             getPaletteColor( sal_Int32 nIndex ) const;
    /** Returns the specified font object. */
    FontRef             getFont( sal_Int32 nFontId ) const;
    /** Returns the specified border object. */
    BorderRef           getBorder( sal_Int32 nBorderId ) const;
    /** Returns the specified cell format object. */
    XfRef               getCellXf( sal_Int32 nXfId ) const;
    /** Returns the specified style format object. */
    XfRef               getStyleXf( sal_Int32 nXfId ) const;

    /** Returns the font object of the specified cell XF. */
    FontRef             getFontFromCellXf( sal_Int32 nXfId ) const;
    /** Returns the default application font (used in the "Normal" cell style). */
    FontRef             getDefaultFont() const;
    /** Returns the model of the default application font (used in the "Normal" cell style). */
    const FontModel&    getDefaultFontModel() const;

    /** Returns true, if the specified borders are equal. */
    static bool         equalBorders( sal_Int32 nBorderId1, sal_Int32 nBorderId2 );
    /** Returns true, if the specified fills are equal. */
    static bool         equalFills( sal_Int32 nFillId1, sal_Int32 nFillId2 );

    /** Returns the default style sheet for unused cells. */
    OUString     getDefaultStyleName() const;
    /** Creates the style sheet described by the style XF with the passed identifier. */
    OUString     createCellStyle( sal_Int32 nXfId ) const;
    ::ScStyleSheet*     getCellStyleSheet( sal_Int32 nXfId ) const;
    /** Creates the style sheet described by the DXF with the passed identifier. */
    OUString     createDxfStyle( sal_Int32 nDxfId ) const;

    void                writeFontToItemSet( SfxItemSet& rItemSet, sal_Int32 nFontId, bool bSkipPoolDefs ) const;
    sal_uInt32          writeNumFmtToItemSet( SfxItemSet& rItemSet, sal_uInt32 nNumFmtId, bool bSkipPoolDefs ) const;
    /** Writes the specified number format to the passed property map. */
    void                writeBorderToItemSet( SfxItemSet& rItemSet, sal_Int32 nBorderId, bool bSkipPoolDefs ) const;
    /** Writes the fill attributes of the specified fill data to the passed property map. */
    void                writeFillToItemSet( SfxItemSet& rItemSet, sal_Int32 nFillId, bool bSkipPoolDefs ) const;

    /** Writes the cell formatting attributes of the specified XF to the passed property set. */
    void                writeCellXfToDoc( ScDocumentImport& rDoc, const ScRange& rRange, sal_Int32 nXfId ) const;

private:
    typedef RefVector< Font >                           FontVector;
    typedef RefVector< Border >                         BorderVector;
    typedef RefVector< Fill >                           FillVector;
    typedef RefVector< Xf >                             XfVector;
    typedef RefVector< Dxf >                            DxfVector;
    typedef ::std::map< sal_Int32, OUString >    DxfStyleMap;

    ColorPalette        maPalette;          /// Color palette.
    FontVector          maFonts;            /// List of font objects.
    NumberFormatsBuffer maNumFmts;          /// List of all number format codes.
    BorderVector        maBorders;          /// List of cell border objects.
    FillVector          maFills;            /// List of cell area fill objects.
    XfVector            maCellXfs;          /// List of cell formats.
    XfVector            maStyleXfs;         /// List of cell styles.
    CellStyleBuffer     maCellStyles;       /// All built-in and user defined cell styles.
    DxfVector           maDxfs;             /// List of differential cell styles.
    mutable DxfStyleMap maDxfStyles;        /// Maps DXF identifiers to Calc style sheet names.
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
