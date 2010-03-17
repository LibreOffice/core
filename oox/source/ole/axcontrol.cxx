/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/ole/axcontrol.hxx"
#include <rtl/tencinfo.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include "properties.hxx"
#include "tokens.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/ole/axbinaryreader.hxx"
#include "oox/ole/axcontrolhelper.hxx"
#include "oox/ole/olehelper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::XFormComponent;
using ::oox::core::FilterBase;

namespace oox {
namespace ole {

// ============================================================================

namespace {

const sal_uInt32 AX_FLAGS_ENABLED           = 0x00000002;
const sal_uInt32 AX_FLAGS_LOCKED            = 0x00000004;
const sal_uInt32 AX_FLAGS_OPAQUE            = 0x00000008;
const sal_uInt32 AX_FLAGS_COLUMNHEADS       = 0x00000400;
const sal_uInt32 AX_FLAGS_ENTIREROWS        = 0x00000800;
const sal_uInt32 AX_FLAGS_EXISTINGENTRIES   = 0x00001000;
const sal_uInt32 AX_FLAGS_CAPTIONLEFT       = 0x00002000;
const sal_uInt32 AX_FLAGS_EDITABLE          = 0x00004000;
const sal_uInt32 AX_FLAGS_IMEMODE_MASK      = 0x00078000;
const sal_uInt32 AX_FLAGS_DRAGENABLED       = 0x00080000;
const sal_uInt32 AX_FLAGS_ENTERASNEWLINE    = 0x00100000;
const sal_uInt32 AX_FLAGS_KEEPSELECTION     = 0x00200000;
const sal_uInt32 AX_FLAGS_TABASCHARACTER    = 0x00400000;
const sal_uInt32 AX_FLAGS_WORDWRAP          = 0x00800000;
const sal_uInt32 AX_FLAGS_BORDERSSUPPRESSED = 0x02000000;
const sal_uInt32 AX_FLAGS_SELECTLINE        = 0x04000000;
const sal_uInt32 AX_FLAGS_SINGLECHARSELECT  = 0x08000000;
const sal_uInt32 AX_FLAGS_AUTOSIZE          = 0x10000000;
const sal_uInt32 AX_FLAGS_HIDESELECTION     = 0x20000000;
const sal_uInt32 AX_FLAGS_MAXLENAUTOTAB     = 0x40000000;
const sal_uInt32 AX_FLAGS_MULTILINE         = 0x80000000;

const sal_uInt32 AX_CMDBUTTON_DEFFLAGS      = 0x0000001B;
const sal_uInt32 AX_LABEL_DEFFLAGS          = 0x0080001B;
const sal_uInt32 AX_IMAGE_DEFFLAGS          = 0x0000001B;
const sal_uInt32 AX_MORPHDATA_DEFFLAGS      = 0x2C80081B;
const sal_uInt32 AX_SPINBUTTON_DEFFLAGS     = 0x0000001B;
const sal_uInt32 AX_SCROLLBAR_DEFFLAGS      = 0x0000001B;

const sal_uInt32 AX_FONT_BOLD               = 0x00000001;
const sal_uInt32 AX_FONT_ITALIC             = 0x00000002;
const sal_uInt32 AX_FONT_UNDERLINE          = 0x00000004;
const sal_uInt32 AX_FONT_STRIKEOUT          = 0x00000008;
const sal_uInt32 AX_FONT_DISABLED           = 0x00002000;
const sal_uInt32 AX_FONT_AUTOCOLOR          = 0x40000000;

const sal_Int32 AX_FONTALIGN_LEFT           = 1;
const sal_Int32 AX_FONTALIGN_RIGHT          = 2;
const sal_Int32 AX_FONTALIGN_CENTER         = 3;

const sal_Int32 AX_BORDERSTYLE_NONE         = 0;
const sal_Int32 AX_BORDERSTYLE_SINGLE       = 1;

const sal_Int32 AX_SPECIALEFFECT_FLAT       = 0;
const sal_Int32 AX_SPECIALEFFECT_RAISED     = 1;
const sal_Int32 AX_SPECIALEFFECT_SUNKEN     = 2;
const sal_Int32 AX_SPECIALEFFECT_ETCHED     = 3;
const sal_Int32 AX_SPECIALEFFECT_BUMPED     = 6;

const sal_uInt16 AX_POS_TOPLEFT             = 0;
const sal_uInt16 AX_POS_TOP                 = 1;
const sal_uInt16 AX_POS_TOPRIGHT            = 2;
const sal_uInt16 AX_POS_LEFT                = 3;
const sal_uInt16 AX_POS_CENTER              = 4;
const sal_uInt16 AX_POS_RIGHT               = 5;
const sal_uInt16 AX_POS_BOTTOMLEFT          = 6;
const sal_uInt16 AX_POS_BOTTOM              = 7;
const sal_uInt16 AX_POS_BOTTOMRIGHT         = 8;

#define AX_PICPOS( label, image ) ((AX_POS_##label << 16) | AX_POS_##image)
const sal_uInt32 AX_PICPOS_LEFTTOP          = AX_PICPOS( TOPRIGHT,    TOPLEFT );
const sal_uInt32 AX_PICPOS_LEFTCENTER       = AX_PICPOS( RIGHT,       LEFT );
const sal_uInt32 AX_PICPOS_LEFTBOTTOM       = AX_PICPOS( BOTTOMRIGHT, BOTTOMLEFT );
const sal_uInt32 AX_PICPOS_RIGHTTOP         = AX_PICPOS( TOPLEFT,     TOPRIGHT );
const sal_uInt32 AX_PICPOS_RIGHTCENTER      = AX_PICPOS( LEFT,        RIGHT );
const sal_uInt32 AX_PICPOS_RIGHTBOTTOM      = AX_PICPOS( BOTTOMLEFT,  BOTTOMRIGHT );
const sal_uInt32 AX_PICPOS_ABOVELEFT        = AX_PICPOS( BOTTOMLEFT,  TOPLEFT );
const sal_uInt32 AX_PICPOS_ABOVECENTER      = AX_PICPOS( BOTTOM,      TOP  );
const sal_uInt32 AX_PICPOS_ABOVERIGHT       = AX_PICPOS( BOTTOMRIGHT, TOPRIGHT );
const sal_uInt32 AX_PICPOS_BELOWLEFT        = AX_PICPOS( TOPLEFT,     BOTTOMLEFT );
const sal_uInt32 AX_PICPOS_BELOWCENTER      = AX_PICPOS( TOP,         BOTTOM );
const sal_uInt32 AX_PICPOS_BELOWRIGHT       = AX_PICPOS( TOPRIGHT,    BOTTOMRIGHT );
const sal_uInt32 AX_PICPOS_CENTER           = AX_PICPOS( CENTER,      CENTER  );
#undef AX_PICPOS

const sal_Int32 AX_PICSIZE_CLIP             = 0;
const sal_Int32 AX_PICSIZE_STRETCH          = 1;
const sal_Int32 AX_PICSIZE_ZOOM             = 3;

const sal_Int32 AX_PICALIGN_TOPLEFT         = 0;
const sal_Int32 AX_PICALIGN_TOPRIGHT        = 1;
const sal_Int32 AX_PICALIGN_CENTER          = 2;
const sal_Int32 AX_PICALIGN_BOTTOMLEFT      = 3;
const sal_Int32 AX_PICALIGN_BOTTOMRIGHT     = 4;

const sal_Int32 AX_DISPLAYSTYLE_TEXT        = 1;
const sal_Int32 AX_DISPLAYSTYLE_LISTBOX     = 2;
const sal_Int32 AX_DISPLAYSTYLE_COMBOBOX    = 3;
const sal_Int32 AX_DISPLAYSTYLE_CHECKBOX    = 4;
const sal_Int32 AX_DISPLAYSTYLE_OPTBUTTON   = 5;
const sal_Int32 AX_DISPLAYSTYLE_TOGGLE      = 6;
const sal_Int32 AX_DISPLAYSTYLE_DROPDOWN    = 7;

const sal_Int32 AX_SELCTION_SINGLE          = 0;
const sal_Int32 AX_SELCTION_MULTI           = 1;
const sal_Int32 AX_SELCTION_EXTENDED        = 2;

const sal_Int32 AX_SCROLLBAR_NONE           = 0x00;
const sal_Int32 AX_SCROLLBAR_HORIZONTAL     = 0x01;
const sal_Int32 AX_SCROLLBAR_VERTICAL       = 0x02;

const sal_Int32 AX_MATCHENTRY_FIRSTLETTER   = 0;
const sal_Int32 AX_MATCHENTRY_COMPLETE      = 1;
const sal_Int32 AX_MATCHENTRY_NONE          = 2;

const sal_Int32 AX_SHOWDROPBUTTON_NEVER     = 0;
const sal_Int32 AX_SHOWDROPBUTTON_FOCUS     = 1;
const sal_Int32 AX_SHOWDROPBUTTON_ALWAYS    = 2;

const sal_Int32 AX_ORIENTATION_AUTO         = -1;
const sal_Int32 AX_ORIENTATION_VERTICAL     = 0;
const sal_Int32 AX_ORIENTATION_HORIZONTAL   = 1;

const sal_Int32 AX_PROPTHUMB_ON             = -1;
const sal_Int32 AX_PROPTHUMB_OFF            = 0;

// ----------------------------------------------------------------------------

const sal_Int16 API_BORDER_NONE             = 0;
const sal_Int16 API_BORDER_SUNKEN           = 1;
const sal_Int16 API_BORDER_FLAT             = 2;

const sal_Int16 API_STATE_UNCHECKED         = 0;
const sal_Int16 API_STATE_CHECKED           = 1;
const sal_Int16 API_STATE_DONTKNOW          = 2;

// ----------------------------------------------------------------------------

/** Specifies how a form control supports transparent background. */
enum ApiTransparencyMode
{
    API_TRANSPARENCY_NOTSUPPORTED,      /// Control does not support transparency.
    API_TRANSPARENCY_VOID,              /// Transparency is enabled by missing fill color.
    API_TRANSPARENCY_PAINTTRANSPARENT   /// Transparency is enabled by the 'PaintTransparent' property.
};

// ----------------------------------------------------------------------------

/** Converts the AX background formatting to UNO properties. */
void lclConvertBackground( AxControlHelper& rHelper, PropertyMap& rPropMap, sal_uInt32 nBackColor, sal_uInt32 nFlags, ApiTransparencyMode eTranspMode )
{
    bool bOpaque = getFlag( nFlags, AX_FLAGS_OPAQUE );
    switch( eTranspMode )
    {
        case API_TRANSPARENCY_NOTSUPPORTED:
            // fake transparency by using system window background if needed
            rPropMap.setProperty( PROP_BackgroundColor, rHelper.convertColor( bOpaque ? nBackColor : AX_SYSCOLOR_WINDOWBACK ) );
        break;
        case API_TRANSPARENCY_PAINTTRANSPARENT:
            rPropMap.setProperty( PROP_PaintTransparent, !bOpaque );
            // run-through intended!
        case API_TRANSPARENCY_VOID:
            // keep transparency by leaving the (void) default property value
            if( bOpaque )
                rPropMap.setProperty( PROP_BackgroundColor, rHelper.convertColor( nBackColor ) );
        break;
    }
}

// ----------------------------------------------------------------------------

/** Converts the AX border formatting to UNO properties. */
void lclConvertBorder( AxControlHelper& rHelper, PropertyMap& rPropMap, sal_uInt32 nBorderColor, sal_Int32 nBorderStyle, sal_Int32 nSpecialEffect )
{
    sal_Int16 nBorder = (nBorderStyle == AX_BORDERSTYLE_SINGLE) ? API_BORDER_FLAT :
        ((nSpecialEffect == AX_SPECIALEFFECT_FLAT) ? API_BORDER_NONE : API_BORDER_SUNKEN);
    rPropMap.setProperty( PROP_Border, nBorder );
    rPropMap.setProperty( PROP_BorderColor, rHelper.convertColor( nBorderColor ) );
}

// ----------------------------------------------------------------------------

/** Converts the AX special effect to UNO properties. */
void lclConvertVisualEffect( AxControlHelper& /*rHelper*/, PropertyMap& rPropMap, sal_Int32 nSpecialEffect )
{
    namespace AwtVisualEffect = ::com::sun::star::awt::VisualEffect;
    sal_Int16 nVisualEffect = (nSpecialEffect == AX_SPECIALEFFECT_FLAT) ? AwtVisualEffect::FLAT : AwtVisualEffect::LOOK3D;
    rPropMap.setProperty( PROP_VisualEffect, nVisualEffect );
}

// ----------------------------------------------------------------------------

/** Converts the passed picture stream to UNO properties. */
void lclConvertPicture( AxControlHelper& rHelper, PropertyMap& rPropMap, const StreamDataSequence& rPicData )
{
    if( rPicData.hasElements() )
    {
        OUString aGraphicUrl = rHelper.getFilter().getGraphicHelper().importGraphicObject( rPicData );
        if( aGraphicUrl.getLength() > 0 )
            rPropMap.setProperty( PROP_ImageURL, aGraphicUrl );
    }
}

// ----------------------------------------------------------------------------

/** Converts the passed picture stream and position to UNO properties. */
void lclConvertPicture( AxControlHelper& rHelper, PropertyMap& rPropMap, const StreamDataSequence& rPicData, sal_uInt32 nPicPos )
{
    // the picture
    lclConvertPicture( rHelper, rPropMap, rPicData );

    // picture position
    namespace AwtImagePos = ::com::sun::star::awt::ImagePosition;
    sal_Int16 nImagePos = AwtImagePos::LeftCenter;
    switch( nPicPos )
    {
        case AX_PICPOS_LEFTTOP:     nImagePos = AwtImagePos::LeftTop;       break;
        case AX_PICPOS_LEFTCENTER:  nImagePos = AwtImagePos::LeftCenter;    break;
        case AX_PICPOS_LEFTBOTTOM:  nImagePos = AwtImagePos::LeftBottom;    break;
        case AX_PICPOS_RIGHTTOP:    nImagePos = AwtImagePos::RightTop;      break;
        case AX_PICPOS_RIGHTCENTER: nImagePos = AwtImagePos::RightCenter;   break;
        case AX_PICPOS_RIGHTBOTTOM: nImagePos = AwtImagePos::RightBottom;   break;
        case AX_PICPOS_ABOVELEFT:   nImagePos = AwtImagePos::AboveLeft;     break;
        case AX_PICPOS_ABOVECENTER: nImagePos = AwtImagePos::AboveCenter;   break;
        case AX_PICPOS_ABOVERIGHT:  nImagePos = AwtImagePos::AboveRight;    break;
        case AX_PICPOS_BELOWLEFT:   nImagePos = AwtImagePos::BelowLeft;     break;
        case AX_PICPOS_BELOWCENTER: nImagePos = AwtImagePos::BelowCenter;   break;
        case AX_PICPOS_BELOWRIGHT:  nImagePos = AwtImagePos::BelowRight;    break;
        case AX_PICPOS_CENTER:      nImagePos = AwtImagePos::Centered;      break;
        default:    OSL_ENSURE( false, "lclConvertPicture - unknown picture position" );
    }
    rPropMap.setProperty( PROP_ImagePosition, nImagePos );
}

// ----------------------------------------------------------------------------

/** Converts the passed picture stream and position to UNO properties. */
void lclConvertPicture( AxControlHelper& rHelper, PropertyMap& rPropMap, const StreamDataSequence& rPicData, sal_Int32 nPicSizeMode, sal_Int32 /*nPicAlign*/, bool /*bPicTiling*/ )
{
    // the picture
    lclConvertPicture( rHelper, rPropMap, rPicData );

    // picture scale mode
    namespace AwtScaleMode = ::com::sun::star::awt::ImageScaleMode;
    sal_Int16 nScaleMode = AwtScaleMode::None;
    switch( nPicSizeMode )
    {
        case AX_PICSIZE_CLIP:       nScaleMode = AwtScaleMode::None;        break;
        case AX_PICSIZE_STRETCH:    nScaleMode = AwtScaleMode::Anisotropic; break;
        case AX_PICSIZE_ZOOM:       nScaleMode = AwtScaleMode::Isotropic;   break;
        default:    OSL_ENSURE( false, "lclConvertPicture - unknown picture size mode" );
    }
    rPropMap.setProperty( PROP_ScaleMode, nScaleMode );
}

// ----------------------------------------------------------------------------

/** Converts the AX value for checked/unchecked/dontknow to UNO properties. */
void lclConvertState( AxControlHelper& /*rHelper*/, PropertyMap& rPropMap, const OUString& rValue, sal_Int32 nMultiSelect, bool bSupportsTriState )
{
    // state
    sal_Int16 nState = bSupportsTriState ? API_STATE_DONTKNOW : API_STATE_UNCHECKED;
    if( rValue.getLength() == 1 ) switch( rValue[ 0 ] )
    {
        case '0':   nState = API_STATE_UNCHECKED;   break;
        case '1':   nState = API_STATE_CHECKED;     break;
        // any other string (also empty) means 'dontknow'
    }
    rPropMap.setProperty( PROP_DefaultState, nState );

    // tristate
    if( bSupportsTriState )
        rPropMap.setProperty( PROP_TriState, nMultiSelect == AX_SELCTION_MULTI );
}

// ----------------------------------------------------------------------------

/** Converts the AX control orientation to UNO properties. */
void lclConvertOrientation( AxControlHelper& /*rHelper*/, PropertyMap& rPropMap, sal_Int32 nOrientation, sal_Int32 nWidth, sal_Int32 nHeight )
{
    namespace AwtScrollBarOrient = ::com::sun::star::awt::ScrollBarOrientation;
    sal_Int32 nScrollOrient = AwtScrollBarOrient::HORIZONTAL;
    switch( nOrientation )
    {
        case AX_ORIENTATION_AUTO:       if( nWidth <= nHeight) nScrollOrient = AwtScrollBarOrient::VERTICAL;    break;
        case AX_ORIENTATION_VERTICAL:   nScrollOrient = AwtScrollBarOrient::VERTICAL;                           break;
        case AX_ORIENTATION_HORIZONTAL: nScrollOrient = AwtScrollBarOrient::HORIZONTAL;                         break;
        default:    OSL_ENSURE( false, "lclConvertOrientation - unknown orientation" );
    }
    rPropMap.setProperty( PROP_Orientation, nScrollOrient );
}

} // namespace

// ============================================================================

AxControlModelBase::AxControlModelBase() :
    mnWidth( 0 ),
    mnHeight( 0 )
{
}

AxControlModelBase::~AxControlModelBase()
{
}

void AxControlModelBase::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        // size of the control shape: format is "width;height"
        case XML_Size:
        {
            sal_Int32 nSepPos = rValue.indexOf( ';' );
            OSL_ENSURE( nSepPos >= 0, "AxControlModelBase::importProperty - missing separator in 'Size' property" );
            if( nSepPos >= 0 )
            {
                mnWidth = rValue.copy( 0, nSepPos ).toInt32();
                mnHeight = rValue.copy( nSepPos + 1 ).toInt32();
            }
        }
        break;
    }
}

void AxControlModelBase::importBinaryModel( BinaryInputStream& /*rInStrm*/ )
{
}

void AxControlModelBase::importPictureData( sal_Int32 /*nPropId*/, BinaryInputStream& /*rInStrm*/ )
{
}

void AxControlModelBase::convertProperties( AxControlHelper& /*rHelper*/, PropertyMap& /*rPropMap*/ ) const
{
}

// ============================================================================

AxFontDataModel::AxFontDataModel() :
    mnFontEffects( 0 ),
    mnFontHeight( 160 ),
    mnFontCharSet( 1 ),
    mnHorAlign( AX_FONTALIGN_LEFT )
{
}

void AxFontDataModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_FontName:          maFontName = rValue;                                        break;
        case XML_FontEffects:       mnFontEffects = AttributeList::decodeUnsigned( rValue );    break;
        case XML_FontHeight:        mnFontHeight = AttributeList::decodeInteger( rValue );      break;
        case XML_FontCharSet:       mnFontCharSet = AttributeList::decodeInteger( rValue );     break;
        case XML_ParagraphAlign:    mnHorAlign = AttributeList::decodeInteger( rValue );        break;
        default:                    AxControlModelBase::importProperty( nPropId, rValue );
    }
}

void AxFontDataModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readStringProperty( maFontName );
    aReader.readIntProperty< sal_uInt32 >( mnFontEffects );
    aReader.readIntProperty< sal_Int32 >( mnFontHeight );
    aReader.skipIntProperty< sal_Int32 >(); // font offset
    aReader.readIntProperty< sal_uInt8 >( mnFontCharSet );
    aReader.skipIntProperty< sal_uInt8 >(); // font pitch/family
    aReader.readIntProperty< sal_uInt8 >( mnHorAlign );
    aReader.skipIntProperty< sal_uInt16 >(); // font weight
    aReader.finalizeImport();
}

void AxFontDataModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    namespace cssa = ::com::sun::star::awt;

    // font name
    if( maFontName.getLength() > 0 )
        rPropMap.setProperty( PROP_FontName, maFontName );

    // font effects
    rPropMap.setProperty( PROP_FontWeight, getFlagValue( mnFontEffects, AX_FONT_BOLD, cssa::FontWeight::BOLD, cssa::FontWeight::NORMAL ) );
    rPropMap.setProperty( PROP_FontSlant, getFlagValue< sal_Int16 >( mnFontEffects, AX_FONT_ITALIC, cssa::FontSlant_ITALIC, cssa::FontSlant_NONE ) );
    rPropMap.setProperty( PROP_FontUnderline, getFlagValue( mnFontEffects, AX_FONT_UNDERLINE, cssa::FontUnderline::SINGLE, cssa::FontUnderline::NONE ) );
    rPropMap.setProperty( PROP_FontStrikeout, getFlagValue( mnFontEffects, AX_FONT_STRIKEOUT, cssa::FontStrikeout::SINGLE, cssa::FontStrikeout::NONE ) );

    /*  font height in points. MSO uses weird font sizes:
        1pt->30, 2pt->45, 3pt->60, 4pt->75, 5pt->105, 6pt->120, 7pt->135,
        8pt->165, 9pt->180, 10pt->195, 11pt->225, ... */
    sal_Int16 nHeight = getLimitedValue< sal_Int16, sal_Int32 >( (mnFontHeight + 10) / 20, 1, SAL_MAX_INT16 );
    rPropMap.setProperty( PROP_FontHeight, nHeight );

    // font character set
    rtl_TextEncoding eFontEnc = RTL_TEXTENCODING_DONTKNOW;
    if( (0 <= mnFontCharSet) && (mnFontCharSet <= SAL_MAX_UINT8) )
        eFontEnc = rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( mnFontCharSet ) );
    if( eFontEnc != RTL_TEXTENCODING_DONTKNOW )
        rPropMap.setProperty( PROP_FontCharset, static_cast< sal_Int16 >( eFontEnc ) );

    // text alignment
    sal_Int32 nAlign = cssa::TextAlign::LEFT;
    switch( mnHorAlign )
    {
        case AX_FONTALIGN_LEFT:     nAlign = cssa::TextAlign::LEFT;     break;
        case AX_FONTALIGN_RIGHT:    nAlign = cssa::TextAlign::RIGHT;    break;
        case AX_FONTALIGN_CENTER:   nAlign = cssa::TextAlign::CENTER;   break;
        default:                OSL_ENSURE( false, "AxFontDataModel::convertProperties - unknown text alignment" );
    }
    // form controls expect short value
    rPropMap.setProperty( PROP_Align, static_cast< sal_Int16 >( nAlign ) );

    // process base class properties
    AxControlModelBase::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxCommandButtonModel::AxCommandButtonModel() :
    mnTextColor( AX_SYSCOLOR_BUTTONTEXT ),
    mnBackColor( AX_SYSCOLOR_BUTTONFACE ),
    mnFlags( AX_CMDBUTTON_DEFFLAGS ),
    mnPicturePos( AX_PICPOS_ABOVECENTER ),
    mbFocusOnClick( true )
{
}

void AxCommandButtonModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_Caption:               maCaption = rValue;                                             break;
        case XML_ForeColor:             mnTextColor = AttributeList::decodeUnsigned( rValue );          break;
        case XML_BackColor:             mnBackColor = AttributeList::decodeUnsigned( rValue );          break;
        case XML_VariousPropertyBits:   mnFlags = AttributeList::decodeUnsigned( rValue );              break;
        case XML_PicturePosition:       mnPicturePos = AttributeList::decodeUnsigned( rValue );         break;
        case XML_TakeFocusOnClick:      mbFocusOnClick = AttributeList::decodeInteger( rValue ) != 0;   break;
        default:                        AxFontDataModel::importProperty( nPropId, rValue );
    }
}

void AxCommandButtonModel::importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm )
{
    switch( nPropId )
    {
        case XML_Picture:   OleHelper::importStdPic( maPictureData, rInStrm, true );    break;
        default:            AxFontDataModel::importPictureData( nPropId, rInStrm );
    }
}

void AxCommandButtonModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readIntProperty< sal_uInt32 >( mnTextColor );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readStringProperty( maCaption );
    aReader.readIntProperty< sal_uInt32 >( mnPicturePos );
    aReader.readPairProperty( mnWidth, mnHeight );
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.readPictureProperty( maPictureData );
    aReader.skipIntProperty< sal_uInt16 >(); // accelerator
    aReader.readBoolProperty( mbFocusOnClick, true ); // binary flag means "do not take focus"
    aReader.skipPictureProperty(); // mouse icon
    if( aReader.finalizeImport() )
        AxFontDataModel::importBinaryModel( rInStrm );
}

OUString AxCommandButtonModel::getServiceName() const
{
    return CREATE_OUSTRING( "com.sun.star.form.component.CommandButton" );
}

void AxCommandButtonModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_TextColor, rHelper.convertColor( mnTextColor ) );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_FocusOnClick, mbFocusOnClick );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_MIDDLE );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    lclConvertPicture( rHelper, rPropMap, maPictureData, mnPicturePos );
    AxFontDataModel::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxLabelModel::AxLabelModel() :
    mnTextColor( AX_SYSCOLOR_BUTTONTEXT ),
    mnBackColor( AX_SYSCOLOR_BUTTONFACE ),
    mnFlags( AX_LABEL_DEFFLAGS ),
    mnBorderColor( AX_SYSCOLOR_WINDOWFRAME ),
    mnBorderStyle( AX_BORDERSTYLE_NONE ),
    mnSpecialEffect( AX_SPECIALEFFECT_FLAT )
{
}

void AxLabelModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_Caption:               maCaption = rValue;                                             break;
        case XML_ForeColor:             mnTextColor = AttributeList::decodeUnsigned( rValue );          break;
        case XML_BackColor:             mnBackColor = AttributeList::decodeUnsigned( rValue );          break;
        case XML_VariousPropertyBits:   mnFlags = AttributeList::decodeUnsigned( rValue );              break;
        case XML_BorderColor:           mnBorderColor = AttributeList::decodeUnsigned( rValue );        break;
        case XML_BorderStyle:           mnBorderStyle = AttributeList::decodeInteger( rValue );         break;
        case XML_SpecialEffect:         mnSpecialEffect = AttributeList::decodeInteger( rValue );       break;
        default:                        AxFontDataModel::importProperty( nPropId, rValue );
    }
}

void AxLabelModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readIntProperty< sal_uInt32 >( mnTextColor );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readStringProperty( maCaption );
    aReader.skipIntProperty< sal_uInt32 >(); // picture position
    aReader.readPairProperty( mnWidth, mnHeight );
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.readIntProperty< sal_uInt32 >( mnBorderColor );
    aReader.readIntProperty< sal_uInt16 >( mnBorderStyle );
    aReader.readIntProperty< sal_uInt16 >( mnSpecialEffect );
    aReader.skipPictureProperty(); // picture
    aReader.skipIntProperty< sal_uInt16 >(); // accelerator
    aReader.skipPictureProperty(); // mouse icon
    if( aReader.finalizeImport() )
        AxFontDataModel::importBinaryModel( rInStrm );
}

OUString AxLabelModel::getServiceName() const
{
    return CREATE_OUSTRING( "com.sun.star.form.component.FixedText" );
}

void AxLabelModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_TextColor, rHelper.convertColor( mnTextColor ) );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_TOP );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    lclConvertBorder( rHelper, rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxFontDataModel::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxImageModel::AxImageModel() :
    mnBackColor( AX_SYSCOLOR_BUTTONFACE ),
    mnFlags( AX_IMAGE_DEFFLAGS ),
    mnBorderColor( AX_SYSCOLOR_WINDOWFRAME ),
    mnBorderStyle( AX_BORDERSTYLE_SINGLE ),
    mnSpecialEffect( AX_SPECIALEFFECT_FLAT ),
    mnPicSizeMode( AX_PICSIZE_CLIP ),
    mnPicAlign( AX_PICALIGN_CENTER ),
    mbPicTiling( false )
{
}

void AxImageModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_BackColor:             mnBackColor = AttributeList::decodeUnsigned( rValue );      break;
        case XML_VariousPropertyBits:   mnFlags = AttributeList::decodeUnsigned( rValue );          break;
        case XML_BorderColor:           mnBorderColor = AttributeList::decodeUnsigned( rValue );    break;
        case XML_BorderStyle:           mnBorderStyle = AttributeList::decodeInteger( rValue );     break;
        case XML_SpecialEffect:         mnSpecialEffect = AttributeList::decodeInteger( rValue );   break;
        case XML_SizeMode:              mnPicSizeMode = AttributeList::decodeInteger( rValue );     break;
        case XML_PictureAlignment:      mnPicAlign = AttributeList::decodeInteger( rValue );        break;
        case XML_PictureTiling:         mbPicTiling = AttributeList::decodeInteger( rValue ) != 0;  break;
        default:                        AxControlModelBase::importProperty( nPropId, rValue );
    }
}

void AxImageModel::importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm )
{
    switch( nPropId )
    {
        case XML_Picture:   OleHelper::importStdPic( maPictureData, rInStrm, true );    break;
        default:            AxControlModelBase::importPictureData( nPropId, rInStrm );
    }
}

void AxImageModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.skipUndefinedProperty();
    aReader.skipUndefinedProperty();
    aReader.skipBoolProperty(); // auto-size
    aReader.readIntProperty< sal_uInt32 >( mnBorderColor );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt8 >( mnBorderStyle );
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.readIntProperty< sal_uInt8 >( mnPicSizeMode );
    aReader.readIntProperty< sal_uInt8 >( mnSpecialEffect );
    aReader.readPairProperty( mnWidth, mnHeight );
    aReader.readPictureProperty( maPictureData );
    aReader.readIntProperty< sal_uInt8 >( mnPicAlign );
    aReader.readBoolProperty( mbPicTiling );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.skipPictureProperty(); // mouse icon
    aReader.finalizeImport();
}

OUString AxImageModel::getServiceName() const
{
    return CREATE_OUSTRING( "com.sun.star.form.component.DatabaseImageControl" );
}

void AxImageModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    lclConvertBorder( rHelper, rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    lclConvertPicture( rHelper, rPropMap, maPictureData, mnPicSizeMode, mnPicAlign, mbPicTiling );
    AxControlModelBase::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxMorphDataModel::AxMorphDataModel() :
    mnTextColor( AX_SYSCOLOR_WINDOWTEXT ),
    mnBackColor( AX_SYSCOLOR_WINDOWBACK ),
    mnFlags( AX_MORPHDATA_DEFFLAGS ),
    mnPicturePos( AX_PICPOS_ABOVECENTER ),
    mnBorderColor( AX_SYSCOLOR_WINDOWFRAME ),
    mnBorderStyle( AX_BORDERSTYLE_NONE ),
    mnSpecialEffect( AX_SPECIALEFFECT_SUNKEN ),
    mnDisplayStyle( AX_DISPLAYSTYLE_TEXT ),
    mnMultiSelect( AX_SELCTION_SINGLE ),
    mnScrollBars( AX_SCROLLBAR_NONE ),
    mnMatchEntry( AX_MATCHENTRY_NONE ),
    mnShowDropButton( AX_SHOWDROPBUTTON_NEVER ),
    mnMaxLength( 0 ),
    mnPasswordChar( 0 ),
    mnListRows( 8 )
{
}

void AxMorphDataModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_Caption:               maCaption = rValue;                                         break;
        case XML_Value:                 maValue = rValue;                                           break;
        case XML_GroupName:             maGroupName = rValue;                                       break;
        case XML_ForeColor:             mnTextColor = AttributeList::decodeUnsigned( rValue );      break;
        case XML_BackColor:             mnBackColor = AttributeList::decodeUnsigned( rValue );      break;
        case XML_VariousPropertyBits:   mnFlags = AttributeList::decodeUnsigned( rValue );          break;
        case XML_PicturePosition:       mnPicturePos = AttributeList::decodeUnsigned( rValue );     break;
        case XML_BorderColor:           mnBorderColor = AttributeList::decodeUnsigned( rValue );    break;
        case XML_BorderStyle:           mnBorderStyle = AttributeList::decodeInteger( rValue );     break;
        case XML_SpecialEffect:         mnSpecialEffect = AttributeList::decodeInteger( rValue );   break;
        case XML_DisplayStyle:          mnDisplayStyle = AttributeList::decodeInteger( rValue );    break;
        case XML_MultiSelect:           mnMultiSelect = AttributeList::decodeInteger( rValue );     break;
        case XML_ScrollBars:            mnScrollBars = AttributeList::decodeInteger( rValue );      break;
        case XML_MatchEntry:            mnMatchEntry = AttributeList::decodeInteger( rValue );      break;
        case XML_ShowDropButtonWhen:    mnShowDropButton = AttributeList::decodeInteger( rValue );  break;
        case XML_MaxLength:             mnMaxLength = AttributeList::decodeInteger( rValue );       break;
        case XML_PasswordChar:          mnPasswordChar = AttributeList::decodeInteger( rValue );    break;
        case XML_ListRows:              mnListRows = AttributeList::decodeInteger( rValue );        break;
        default:                        AxFontDataModel::importProperty( nPropId, rValue );
    }
}

void AxMorphDataModel::importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm )
{
    switch( nPropId )
    {
        case XML_Picture:   OleHelper::importStdPic( maPictureData, rInStrm, true );    break;
        default:            AxFontDataModel::importPictureData( nPropId, rInStrm );
    }
}

void AxMorphDataModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm, true );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnTextColor );
    aReader.readIntProperty< sal_Int32 >( mnMaxLength );
    aReader.readIntProperty< sal_uInt8 >( mnBorderStyle );
    aReader.readIntProperty< sal_uInt8 >( mnScrollBars );
    aReader.readIntProperty< sal_uInt8 >( mnDisplayStyle );
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.readPairProperty( mnWidth, mnHeight );
    aReader.readIntProperty< sal_uInt16 >( mnPasswordChar );
    aReader.skipIntProperty< sal_uInt32 >(); // list width
    aReader.skipIntProperty< sal_uInt16 >(); // bound column
    aReader.skipIntProperty< sal_Int16 >(); // text column
    aReader.skipIntProperty< sal_Int16 >(); // column count
    aReader.readIntProperty< sal_uInt16 >( mnListRows );
    aReader.skipIntProperty< sal_uInt16 >(); // column info count
    aReader.readIntProperty< sal_uInt8 >( mnMatchEntry );
    aReader.skipIntProperty< sal_uInt8 >(); // list style
    aReader.readIntProperty< sal_uInt8 >( mnShowDropButton );
    aReader.skipUndefinedProperty();
    aReader.skipIntProperty< sal_uInt8 >(); // drop down style
    aReader.readIntProperty< sal_uInt8 >( mnMultiSelect );
    aReader.readStringProperty( maValue );
    aReader.readStringProperty( maCaption );
    aReader.readIntProperty< sal_uInt32 >( mnPicturePos );
    aReader.readIntProperty< sal_uInt32 >( mnBorderColor );
    aReader.readIntProperty< sal_uInt32 >( mnSpecialEffect );
    aReader.skipPictureProperty(); // mouse icon
    aReader.readPictureProperty( maPictureData );
    aReader.skipIntProperty< sal_uInt16 >(); // accelerator
    aReader.skipUndefinedProperty();
    aReader.skipBoolProperty();
    aReader.readStringProperty( maGroupName );
    if( aReader.finalizeImport() )
        AxFontDataModel::importBinaryModel( rInStrm );
}

void AxMorphDataModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    rPropMap.setProperty( PROP_TextColor, rHelper.convertColor( mnTextColor ) );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    AxFontDataModel::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxToggleButtonModel::AxToggleButtonModel()
{
}

OUString AxToggleButtonModel::getServiceName() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_TOGGLE, "AxToggleButtonModel::getServiceName - invalid control type" );
    return CREATE_OUSTRING( "com.sun.star.form.component.CommandButton" );
}

void AxToggleButtonModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_MIDDLE );
    rPropMap.setProperty( PROP_Toggle, true );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    lclConvertPicture( rHelper, rPropMap, maPictureData, mnPicturePos );
    AxMorphDataModel::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxCheckBoxModel::AxCheckBoxModel()
{
}

OUString AxCheckBoxModel::getServiceName() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_CHECKBOX, "AxCheckBoxModel::getServiceName - invalid control type" );
    return CREATE_OUSTRING( "com.sun.star.form.component.CheckBox" );
}

void AxCheckBoxModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_MIDDLE );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    lclConvertVisualEffect( rHelper, rPropMap, mnSpecialEffect );
    lclConvertPicture( rHelper, rPropMap, maPictureData, mnPicturePos );
    lclConvertState( rHelper, rPropMap, maValue, mnMultiSelect, true );
    AxMorphDataModel::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxOptionButtonModel::AxOptionButtonModel()
{
}

OUString AxOptionButtonModel::getServiceName() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_OPTBUTTON, "AxOptionButtonModel::getServiceName - invalid control type" );
    return CREATE_OUSTRING( "com.sun.star.form.component.RadioButton" );
}

void AxOptionButtonModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_MIDDLE );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    lclConvertVisualEffect( rHelper, rPropMap, mnSpecialEffect );
    lclConvertPicture( rHelper, rPropMap, maPictureData, mnPicturePos );
    lclConvertState( rHelper, rPropMap, maValue, mnMultiSelect, false );
    AxMorphDataModel::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxTextBoxModel::AxTextBoxModel()
{
}

OUString AxTextBoxModel::getServiceName() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_TEXT, "AxTextBoxModel::getServiceName - invalid control type" );
    return CREATE_OUSTRING( "com.sun.star.form.component.TextField" );
}

void AxTextBoxModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_MULTILINE ) );
    rPropMap.setProperty( PROP_HideInactiveSelection, getFlag( mnFlags, AX_FLAGS_HIDESELECTION ) );
    rPropMap.setProperty( PROP_DefaultText, maValue );
    rPropMap.setProperty( PROP_MaxTextLen, getLimitedValue< sal_Int16, sal_Int32 >( mnMaxLength, 0, SAL_MAX_INT16 ) );
    if( (0 < mnPasswordChar) && (mnPasswordChar <= SAL_MAX_INT16) )
        rPropMap.setProperty( PROP_EchoChar, static_cast< sal_Int16 >( mnPasswordChar ) );
    rPropMap.setProperty( PROP_HScroll, getFlag( mnScrollBars, AX_SCROLLBAR_HORIZONTAL ) );
    rPropMap.setProperty( PROP_VScroll, getFlag( mnScrollBars, AX_SCROLLBAR_VERTICAL ) );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    lclConvertBorder( rHelper, rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModel::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxListBoxModel::AxListBoxModel()
{
}

OUString AxListBoxModel::getServiceName() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_LISTBOX, "AxListBoxModel::getServiceName - invalid control type" );
    return CREATE_OUSTRING( "com.sun.star.form.component.ListBox" );
}

void AxListBoxModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    bool bMultiSelect = (mnMultiSelect == AX_SELCTION_MULTI) || (mnMultiSelect == AX_SELCTION_EXTENDED);
    rPropMap.setProperty( PROP_MultiSelection, bMultiSelect );
    rPropMap.setProperty( PROP_Dropdown, false );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    lclConvertBorder( rHelper, rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModel::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxComboBoxModel::AxComboBoxModel()
{
}

OUString AxComboBoxModel::getServiceName() const
{
    OSL_ENSURE( (mnDisplayStyle == AX_DISPLAYSTYLE_COMBOBOX) || (mnDisplayStyle == AX_DISPLAYSTYLE_DROPDOWN), "AxComboBoxModel::getServiceName - invalid control type" );
    return (mnDisplayStyle == AX_DISPLAYSTYLE_DROPDOWN) ? CREATE_OUSTRING( "com.sun.star.form.component.ListBox" ) : CREATE_OUSTRING( "com.sun.star.form.component.ComboBox" );
}

void AxComboBoxModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    if( mnDisplayStyle != AX_DISPLAYSTYLE_DROPDOWN )
    {
        rPropMap.setProperty( PROP_HideInactiveSelection, getFlag( mnFlags, AX_FLAGS_HIDESELECTION ) );
        rPropMap.setProperty( PROP_DefaultText, maValue );
        rPropMap.setProperty( PROP_MaxTextLen, getLimitedValue< sal_Int16, sal_Int32 >( mnMaxLength, 0, SAL_MAX_INT16 ) );
        bool bAutoComplete = (mnMatchEntry == AX_MATCHENTRY_FIRSTLETTER) || (mnMatchEntry == AX_MATCHENTRY_COMPLETE);
        rPropMap.setProperty( PROP_Autocomplete, bAutoComplete );
    }
    bool bShowDropdown = (mnShowDropButton == AX_SHOWDROPBUTTON_FOCUS) || (mnShowDropButton == AX_SHOWDROPBUTTON_ALWAYS);
    rPropMap.setProperty( PROP_Dropdown, bShowDropdown );
    rPropMap.setProperty( PROP_LineCount, getLimitedValue< sal_Int16, sal_Int32 >( mnListRows, 1, SAL_MAX_INT16 ) );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    lclConvertBorder( rHelper, rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModel::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxSpinButtonModel::AxSpinButtonModel() :
    mnArrowColor( AX_SYSCOLOR_BUTTONTEXT ),
    mnBackColor( AX_SYSCOLOR_BUTTONFACE ),
    mnFlags( AX_SPINBUTTON_DEFFLAGS ),
    mnOrientation( AX_ORIENTATION_AUTO ),
    mnMin( 0 ),
    mnMax( 100 ),
    mnPosition( 0 ),
    mnSmallChange( 1 ),
    mnDelay( 50 )
{
}

OUString AxSpinButtonModel::getServiceName() const
{
    return CREATE_OUSTRING( "com.sun.star.form.component.SpinButton" );
}

void AxSpinButtonModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_ForeColor:             mnArrowColor = AttributeList::decodeUnsigned( rValue ); break;
        case XML_BackColor:             mnBackColor = AttributeList::decodeUnsigned( rValue );  break;
        case XML_VariousPropertyBits:   mnFlags = AttributeList::decodeUnsigned( rValue );      break;
        case XML_Orientation:           mnOrientation = AttributeList::decodeInteger( rValue ); break;
        case XML_Min:                   mnMin = AttributeList::decodeInteger( rValue );         break;
        case XML_Max:                   mnMax = AttributeList::decodeInteger( rValue );         break;
        case XML_Position:              mnPosition = AttributeList::decodeInteger( rValue );    break;
        case XML_SmallChange:           mnSmallChange = AttributeList::decodeInteger( rValue ); break;
        case XML_Delay:                 mnDelay = AttributeList::decodeInteger( rValue );       break;
        default:                        AxControlModelBase::importProperty( nPropId, rValue );
    }
}

void AxSpinButtonModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readIntProperty< sal_uInt32 >( mnArrowColor );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readPairProperty( mnWidth, mnHeight );
    aReader.skipIntProperty< sal_uInt32 >(); // unused
    aReader.readIntProperty< sal_Int32 >( mnMin );
    aReader.readIntProperty< sal_Int32 >( mnMax );
    aReader.readIntProperty< sal_Int32 >( mnPosition );
    aReader.skipIntProperty< sal_uInt32 >(); // prev enabled
    aReader.skipIntProperty< sal_uInt32 >(); // next enabled
    aReader.readIntProperty< sal_Int32 >( mnSmallChange );
    aReader.readIntProperty< sal_Int32 >( mnOrientation );
    aReader.readIntProperty< sal_Int32 >( mnDelay );
    aReader.skipPictureProperty(); // mouse icon
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.finalizeImport();
}

void AxSpinButtonModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    sal_Int32 nMin = ::std::min( mnMin, mnMax );
    sal_Int32 nMax = ::std::max( mnMin, mnMax );
    rPropMap.setProperty( PROP_SymbolColor, rHelper.convertColor( mnArrowColor ) );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rPropMap.setProperty( PROP_SpinValueMin, nMin );
    rPropMap.setProperty( PROP_SpinValueMax, nMax );
    rPropMap.setProperty( PROP_SpinIncrement, mnSmallChange );
    rPropMap.setProperty( PROP_DefaultSpinValue, mnPosition );
    rPropMap.setProperty( PROP_Repeat, true );
    rPropMap.setProperty( PROP_RepeatDelay, mnDelay );
    rPropMap.setProperty( PROP_Border, API_BORDER_NONE );
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    lclConvertOrientation( rHelper, rPropMap, mnOrientation, mnWidth, mnHeight );
    AxControlModelBase::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxScrollBarModel::AxScrollBarModel() :
    mnArrowColor( AX_SYSCOLOR_BUTTONTEXT ),
    mnBackColor( AX_SYSCOLOR_BUTTONFACE ),
    mnFlags( AX_SCROLLBAR_DEFFLAGS ),
    mnOrientation( AX_ORIENTATION_AUTO ),
    mnPropThumb( AX_PROPTHUMB_ON ),
    mnMin( 0 ),
    mnMax( 32767 ),
    mnPosition( 0 ),
    mnSmallChange( 1 ),
    mnLargeChange( 1 ),
    mnDelay( 50 )
{
}

OUString AxScrollBarModel::getServiceName() const
{
    return CREATE_OUSTRING( "com.sun.star.form.component.ScrollBar" );
}

void AxScrollBarModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_ForeColor:             mnArrowColor = AttributeList::decodeUnsigned( rValue ); break;
        case XML_BackColor:             mnBackColor = AttributeList::decodeUnsigned( rValue );  break;
        case XML_VariousPropertyBits:   mnFlags = AttributeList::decodeUnsigned( rValue );      break;
        case XML_Orientation:           mnOrientation = AttributeList::decodeInteger( rValue ); break;
        case XML_ProportionalThumb:     mnPropThumb = AttributeList::decodeInteger( rValue );   break;
        case XML_Min:                   mnMin = AttributeList::decodeInteger( rValue );         break;
        case XML_Max:                   mnMax = AttributeList::decodeInteger( rValue );         break;
        case XML_Position:              mnPosition = AttributeList::decodeInteger( rValue );    break;
        case XML_SmallChange:           mnSmallChange = AttributeList::decodeInteger( rValue ); break;
        case XML_LargeChange:           mnLargeChange = AttributeList::decodeInteger( rValue ); break;
        case XML_Delay:                 mnDelay = AttributeList::decodeInteger( rValue );       break;
        default:                        AxControlModelBase::importProperty( nPropId, rValue );
    }
}

void AxScrollBarModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readIntProperty< sal_uInt32 >( mnArrowColor );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readPairProperty( mnWidth, mnHeight );
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.readIntProperty< sal_Int32 >( mnMin );
    aReader.readIntProperty< sal_Int32 >( mnMax );
    aReader.readIntProperty< sal_Int32 >( mnPosition );
    aReader.skipIntProperty< sal_uInt32 >(); // unused
    aReader.skipIntProperty< sal_uInt32 >(); // prev enabled
    aReader.skipIntProperty< sal_uInt32 >(); // next enabled
    aReader.readIntProperty< sal_Int32 >( mnSmallChange );
    aReader.readIntProperty< sal_Int32 >( mnLargeChange );
    aReader.readIntProperty< sal_Int32 >( mnOrientation );
    aReader.readIntProperty< sal_Int16 >( mnPropThumb );
    aReader.readIntProperty< sal_Int32 >( mnDelay );
    aReader.skipPictureProperty(); // mouse icon
    aReader.finalizeImport();
}

void AxScrollBarModel::convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const
{
    sal_Int32 nMin = ::std::min( mnMin, mnMax );
    sal_Int32 nMax = ::std::max( mnMin, mnMax );
    rPropMap.setProperty( PROP_SymbolColor, rHelper.convertColor( mnArrowColor ) );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rPropMap.setProperty( PROP_ScrollValueMin, nMin );
    rPropMap.setProperty( PROP_ScrollValueMax, nMax );
    rPropMap.setProperty( PROP_LineIncrement, mnSmallChange );
    rPropMap.setProperty( PROP_BlockIncrement, mnLargeChange );
    rPropMap.setProperty( PROP_DefaultScrollValue, mnPosition );
    rPropMap.setProperty( PROP_RepeatDelay, mnDelay );
    rPropMap.setProperty( PROP_Border, API_BORDER_NONE );
    if( (mnPropThumb == AX_PROPTHUMB_ON) && (nMin < nMax) && (mnLargeChange > 0) )
    {
        double fInterval = nMax - nMin; // prevent integer overflow (fInterval+mnLargeChange may become 0 when int is used)
        sal_Int32 nThumbLen = getLimitedValue< sal_Int32, double >( (fInterval * mnLargeChange) / (fInterval + mnLargeChange), 1, SAL_MAX_INT32 );
        rPropMap.setProperty( PROP_VisibleSize, nThumbLen );
    }
    lclConvertBackground( rHelper, rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    lclConvertOrientation( rHelper, rPropMap, mnOrientation, mnWidth, mnHeight );
    AxControlModelBase::convertProperties( rHelper, rPropMap );
}

// ============================================================================

AxControl::AxControl( const OUString& rName ) :
    maName( rName )
{
}

AxControl::~AxControl()
{
}

AxControlModelBase* AxControl::createModel( const OUString& rClassId )
{
    // TODO: move into a factory
    maClassId = rClassId.toAsciiUpperCase();
    if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{D7053240-CE69-11CD-A777-00DD01143C57}" ) ) )       // Forms.CommandButton.1
        mxModel.reset( new AxCommandButtonModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{978C9E23-D4B0-11CE-BF2D-00AA003F40D0}" ) ) )  // Forms.Label.1
        mxModel.reset( new AxLabelModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{4C599241-6926-101B-9992-00000B65C6F9}" ) ) )  // Forms.Image.1
        mxModel.reset( new AxImageModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{8BD21D60-EC42-11CE-9E0D-00AA006002F3}" ) ) )  // Forms.ToggleButton.1
        mxModel.reset( new AxToggleButtonModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{8BD21D40-EC42-11CE-9E0D-00AA006002F3}" ) ) )  // Forms.CheckBox.1
        mxModel.reset( new AxCheckBoxModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{8BD21D50-EC42-11CE-9E0D-00AA006002F3}" ) ) )  // Forms.OptionButton.1
        mxModel.reset( new AxOptionButtonModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{8BD21D10-EC42-11CE-9E0D-00AA006002F3}" ) ) )  // Forms.TextBox.1
        mxModel.reset( new AxTextBoxModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{8BD21D20-EC42-11CE-9E0D-00AA006002F3}" ) ) )  // Forms.ListBox.1
        mxModel.reset( new AxListBoxModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{8BD21D30-EC42-11CE-9E0D-00AA006002F3}" ) ) )  // Forms.ComboBox.1
        mxModel.reset( new AxComboBoxModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{79176FB0-B7F2-11CE-97EF-00AA006D2776}" ) ) )  // Forms.SpinButton.1
        mxModel.reset( new AxSpinButtonModel );
    else if( maClassId.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "{DFD181E0-5E2F-11CE-A449-00AA004A803D}" ) ) )  // Forms.ScrollBar.1
        mxModel.reset( new AxScrollBarModel );
    else
        mxModel.reset();

    return mxModel.get();
}

void AxControl::importBinaryModel( BinaryInputStream& rInStrm )
{
    if( AxControlModelBase* pModel = createModel( OleHelper::importGuid( rInStrm ) ) )
        pModel->importBinaryModel( rInStrm );
}

Reference< XControlModel > AxControl::convertAndInsert( AxControlHelper& rHelper ) const
{
    Reference< XControlModel > xCtrlModel;
    if( mxModel.get() ) try
    {
        Reference< XIndexContainer > xFormIC( rHelper.getControlForm(), UNO_QUERY_THROW );

        // document model creates the form control model
        xCtrlModel.set( rHelper.getFilter().getModelFactory()->createInstance( mxModel->getServiceName() ), UNO_QUERY_THROW );
        Reference< XFormComponent > xFormComp( xCtrlModel, UNO_QUERY_THROW );

        // insert control model into the passed form
        sal_Int32 nNewIndex = xFormIC->getCount();
        xFormIC->insertByIndex( nNewIndex, Any( xFormComp ) );

        // convert all control properties
        PropertyMap aPropMap;
        aPropMap.setProperty( PROP_Name, maName );
        mxModel->convertProperties( rHelper, aPropMap );

        PropertySet aPropSet( xCtrlModel );
        aPropSet.setProperties( aPropMap );
    }
    catch( Exception& )
    {
        xCtrlModel.clear();     // on error: forget the created form control model
    }
    return xCtrlModel;
}

// ============================================================================

} // namespace ole
} // namespace oox

