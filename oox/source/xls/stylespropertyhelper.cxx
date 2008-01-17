/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stylespropertyhelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 * *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/xls/stylespropertyhelper.hxx"
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontType.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include "oox/helper/propertyset.hxx"
#include "oox/xls/stylesbuffer.hxx"

using ::rtl::OUString;

namespace oox {
namespace xls {

// ============================================================================

ApiFontUsedFlags::ApiFontUsedFlags( bool bAllUsed ) :
    mbNameUsed( bAllUsed ),
    mbColorUsed( bAllUsed ),
    mbSchemeUsed( bAllUsed ),
    mbHeightUsed( bAllUsed ),
    mbUnderlineUsed( bAllUsed ),
    mbEscapementUsed( bAllUsed ),
    mbWeightUsed( bAllUsed ),
    mbPostureUsed( bAllUsed ),
    mbStrikeoutUsed( bAllUsed ),
    mbOutlineUsed( bAllUsed ),
    mbShadowUsed( bAllUsed )
{
}

// ----------------------------------------------------------------------------

ApiFontData::ApiFontData() :
    maDesc(
        CREATE_OUSTRING( "Calibri" ),
        220,                                            // height 11 points
        0,
        OUString(),
        ::com::sun::star::awt::FontFamily::DONTKNOW,
        RTL_TEXTENCODING_DONTKNOW,
        ::com::sun::star::awt::FontPitch::DONTKNOW,
        100.0,
        ::com::sun::star::awt::FontWeight::NORMAL,
        ::com::sun::star::awt::FontSlant_NONE,
        ::com::sun::star::awt::FontUnderline::NONE,
        ::com::sun::star::awt::FontStrikeout::NONE,
        0.0,
        sal_False,
        sal_False,
        ::com::sun::star::awt::FontType::DONTKNOW ),
    mnColor( API_RGB_TRANSPARENT ),
    mnEscapement( API_ESCAPE_NONE ),
    mnEscapeHeight( API_ESCAPEHEIGHT_NONE ),
    mbOutline( false ),
    mbShadow( false ),
    mbHasWstrn( true ),
    mbHasAsian( false )
{
}

// ============================================================================

ApiNumFmtData::ApiNumFmtData() :
    mnIndex( 0 )
{
}

// ============================================================================

ApiAlignmentData::ApiAlignmentData() :
    meHorJustify( ::com::sun::star::table::CellHoriJustify_STANDARD ),
    meVerJustify( ::com::sun::star::table::CellVertJustify_STANDARD ),
    meOrientation( ::com::sun::star::table::CellOrientation_STANDARD ),
    mnRotation( 0 ),
    mnWritingMode( ::com::sun::star::text::WritingMode2::PAGE ),
    mnIndent( 0 ),
    mbWrapText( false ),
    mbShrink( false )
{
}

bool operator==( const ApiAlignmentData& rLeft, const ApiAlignmentData& rRight )
{
    return
        (rLeft.meHorJustify  == rRight.meHorJustify) &&
        (rLeft.meVerJustify  == rRight.meVerJustify) &&
        (rLeft.meOrientation == rRight.meOrientation) &&
        (rLeft.mnRotation    == rRight.mnRotation) &&
        (rLeft.mnWritingMode == rRight.mnWritingMode) &&
        (rLeft.mnIndent      == rRight.mnIndent) &&
        (rLeft.mbWrapText    == rRight.mbWrapText) &&
        (rLeft.mbShrink      == rRight.mbShrink);
}

// ============================================================================

ApiProtectionData::ApiProtectionData() :
    maCellProt( sal_True, sal_False, sal_False, sal_False )
{
}

bool operator==( const ApiProtectionData& rLeft, const ApiProtectionData& rRight )
{
    return
        (rLeft.maCellProt.IsLocked        == rRight.maCellProt.IsLocked) &&
        (rLeft.maCellProt.IsFormulaHidden == rRight.maCellProt.IsFormulaHidden) &&
        (rLeft.maCellProt.IsHidden        == rRight.maCellProt.IsHidden) &&
        (rLeft.maCellProt.IsPrintHidden   == rRight.maCellProt.IsPrintHidden);
}

// ============================================================================

ApiBorderData::ApiBorderData() :
    mbBorderUsed( false ),
    mbDiagUsed( false )
{
}

// ============================================================================

ApiSolidFillData::ApiSolidFillData() :
    mnColor( API_RGB_TRANSPARENT ),
    mbTransparent( true ),
    mbUsed( false )
{
}

// ============================================================================

namespace {

/** Property names for Western font name settings. */
const sal_Char* const sppcWstrnFontNameNames[] =
{
    "CharFontName",
    "CharFontFamily",
    "CharFontCharSet",
    0
};

/** Property names for Asian font name settings. */
const sal_Char* const sppcAsianFontNameNames[] =
{
    "CharFontNameAsian",
    "CharFontFamilyAsian",
    "CharFontCharSetAsian",
    0
};

/** Property names for Complex font name settings. */
const sal_Char* const sppcCmplxFontNameNames[] =
{
    "CharFontNameComplex",
    "CharFontFamilyComplex",
    "CharFontCharSetComplex",
    0
};

/** Property names for font height settings. */
const sal_Char* const sppcFontHeightNames[] =
{
    "CharHeight",
    "CharHeightAsian",
    "CharHeightComplex",
    0
};

/** Property names for font weight settings. */
const sal_Char* const sppcFontWeightNames[] =
{
    "CharWeight",
    "CharWeightAsian",
    "CharWeightComplex",
    0
};

/** Property names for font posture settings. */
const sal_Char* const sppcFontPostureNames[] =
{
    "CharPosture",
    "CharPostureAsian",
    "CharPostureComplex",
    0
};

/** Property names for font escapement settings. */
const sal_Char* const sppcFontEscapeNames[] =
{
    "CharEscapement",
    "CharEscapementHeight",
    0
};

/** Property names for alignment. */
const sal_Char* const sppcAlignmentNames[] =
{
    "HoriJustify",
    "VertJustify",
    "WritingMode",
    "RotateAngle",
    "RotateReference",
    "Orientation",
    "ParaIndent",
    "IsTextWrapped",
    "ShrinkToFit",
    0
};

/** Property names for diagonal cell borders. */
const sal_Char* const sppcDiagBorderNames[] =
{
    "DiagonalTLBR",
    "DiagonalBLTR",
    0
};

/** Property names for cell fill. */
const sal_Char* const sppcSolidFillNames[] =
{
    "CellBackColor",
    "IsCellBackgroundTransparent",
    0
};

} // namespace

// ----------------------------------------------------------------------------

StylesPropertyHelper::StylesPropertyHelper( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maWstrnFontNameProps( sppcWstrnFontNameNames ),
    maAsianFontNameProps( sppcAsianFontNameNames ),
    maCmplxFontNameProps( sppcCmplxFontNameNames ),
    maFontHeightProps( sppcFontHeightNames ),
    maFontWeightProps( sppcFontWeightNames ),
    maFontPostureProps( sppcFontPostureNames ),
    maFontEscapeProps( sppcFontEscapeNames ),
    maAlignProps( sppcAlignmentNames ),
    maDiagBorderProps( sppcDiagBorderNames ),
    maSolidFillProps( sppcSolidFillNames ),
    maCharColorProp( CREATE_OUSTRING( "CharColor" ) ),
    maCharUnderlineProp( CREATE_OUSTRING( "CharUnderline" ) ),
    maCharStrikeoutProp( CREATE_OUSTRING( "CharStrikeout" ) ),
    maCharContouredProp( CREATE_OUSTRING( "CharContoured" ) ),
    maCharShadowedProp( CREATE_OUSTRING( "CharShadowed" ) ),
    maNumFmtProp( CREATE_OUSTRING( "NumberFormat" ) ),
    maCellProtProp( CREATE_OUSTRING( "CellProtection" ) ),
    maBorderProp( CREATE_OUSTRING( "TableBorder" ) )
{
}

void StylesPropertyHelper::writeFontProperties( PropertySet& rPropSet,
        const ApiFontData& rFontData, const ApiFontUsedFlags& rUsedFlags, FontPropertyType ePropType )
{
    // font name properties
    if( rUsedFlags.mbNameUsed )
    {
        if( rFontData.mbHasWstrn )
            maWstrnFontNameProps << rFontData.maDesc.Name << rFontData.maDesc.Family << rFontData.maDesc.CharSet >> rPropSet;
        if( rFontData.mbHasAsian )
            maAsianFontNameProps << rFontData.maDesc.Name << rFontData.maDesc.Family << rFontData.maDesc.CharSet >> rPropSet;
        if( rFontData.mbHasCmplx )
            maCmplxFontNameProps << rFontData.maDesc.Name << rFontData.maDesc.Family << rFontData.maDesc.CharSet >> rPropSet;
    }
    // font height
    if( rUsedFlags.mbHeightUsed )
    {
        float fHeight = static_cast< float >( rFontData.maDesc.Height / 20.0 ); // twips to points
        maFontHeightProps << fHeight << fHeight << fHeight >> rPropSet;
    }
    // font weight
    if( rUsedFlags.mbWeightUsed )
    {
        float fWeight = rFontData.maDesc.Weight;
        maFontWeightProps << fWeight << fWeight << fWeight >> rPropSet;
    }
    // font posture
    if( rUsedFlags.mbPostureUsed )
        maFontPostureProps << rFontData.maDesc.Slant << rFontData.maDesc.Slant << rFontData.maDesc.Slant >> rPropSet;
    // character color
    if( rUsedFlags.mbColorUsed )
        rPropSet.setProperty( maCharColorProp, rFontData.mnColor );
    // underline style
    if( rUsedFlags.mbUnderlineUsed )
        rPropSet.setProperty( maCharUnderlineProp, rFontData.maDesc.Underline );
    // strike out style
    if( rUsedFlags.mbStrikeoutUsed )
        rPropSet.setProperty( maCharStrikeoutProp, rFontData.maDesc.Strikeout );
    // outline style
    if( rUsedFlags.mbOutlineUsed )
        rPropSet.setProperty( maCharContouredProp, rFontData.mbOutline );
    // shadow style
    if( rUsedFlags.mbShadowUsed )
        rPropSet.setProperty( maCharShadowedProp, rFontData.mbShadow );
    // escapement
    if( rUsedFlags.mbEscapementUsed && (ePropType == FONT_PROPTYPE_RICHTEXT) )
        maFontEscapeProps << rFontData.mnEscapement << rFontData.mnEscapeHeight >> rPropSet;
}

void StylesPropertyHelper::writeNumFmtProperties(
        PropertySet& rPropSet, const ApiNumFmtData& rNumFmtData )
{
    rPropSet.setProperty( maNumFmtProp, rNumFmtData.mnIndex );
}

void StylesPropertyHelper::writeAlignmentProperties(
        PropertySet& rPropSet, const ApiAlignmentData& rAlignData )
{
    maAlignProps
        << rAlignData.meHorJustify
        << rAlignData.meVerJustify
        << rAlignData.mnWritingMode
        << rAlignData.mnRotation
        << ::com::sun::star::table::CellVertJustify_STANDARD    // rotation reference
        << rAlignData.meOrientation
        << rAlignData.mnIndent
        << rAlignData.mbWrapText
        << rAlignData.mbShrink
        >> rPropSet;
}

void StylesPropertyHelper::writeProtectionProperties(
        PropertySet& rPropSet, const ApiProtectionData& rProtData )
{
    rPropSet.setProperty( maCellProtProp, rProtData.maCellProt );
}

void StylesPropertyHelper::writeBorderProperties(
        PropertySet& rPropSet, const ApiBorderData& rBorderData )
{
    if( rBorderData.mbBorderUsed )
        rPropSet.setProperty( maBorderProp, rBorderData.maBorder );
    if( rBorderData.mbDiagUsed )
        maDiagBorderProps << rBorderData.maTLtoBR << rBorderData.maBLtoTR >> rPropSet;
}

void StylesPropertyHelper::writeSolidFillProperties(
        PropertySet& rPropSet, const ApiSolidFillData& rFillData )
{
    if( rFillData.mbUsed )
        maSolidFillProps << rFillData.mnColor << rFillData.mbTransparent >> rPropSet;
}

// ============================================================================

} // namespace xls
} // namespace oox

