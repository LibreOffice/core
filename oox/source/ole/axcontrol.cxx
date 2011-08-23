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
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include "properties.hxx"
#include "tokens.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"

using ::rtl::OUString;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Size;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::form::XForm;
using ::com::sun::star::form::XFormComponent;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;

namespace oox {
namespace ole {

// ============================================================================

namespace {

const sal_uInt32 COMCTL_ID_SIZE             = 0x12344321;

const sal_uInt32 COMCTL_ID_COMMONDATA       = 0xABCDEF01;
const sal_uInt32 COMCTL_COMMON_FLATBORDER   = 0x00000001;
const sal_uInt32 COMCTL_COMMON_ENABLED      = 0x00000002;
const sal_uInt32 COMCTL_COMMON_3DBORDER     = 0x00000004;
const sal_uInt32 COMCTL_COMMON_OLEDROPMAN   = 0x00002000;

const sal_uInt32 COMCTL_ID_COMPLEXDATA      = 0xBDECDE1F;
const sal_uInt32 COMCTL_COMPLEX_FONT        = 0x00000001;
const sal_uInt32 COMCTL_COMPLEX_MOUSEICON   = 0x00000002;

const sal_uInt32 COMCTL_ID_SCROLLBAR_60     = 0x99470A83;
const sal_uInt32 COMCTL_SCROLLBAR_HOR       = 0x00000010;
const sal_Int32 COMCTL_SCROLLBAR_3D         = 0;
const sal_Int32 COMCTL_SCROLLBAR_FLAT       = 1;
const sal_Int32 COMCTL_SCROLLBAR_TRACK3D    = 2;

const sal_uInt32 COMCTL_ID_PROGRESSBAR_50   = 0xE6E17E84;
const sal_uInt32 COMCTL_ID_PROGRESSBAR_60   = 0x97AB8A01;

// ----------------------------------------------------------------------------

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
const sal_uInt32 AX_TABSTRIP_DEFFLAGS       = 0x0000001B;

const sal_uInt16 AX_POS_TOPLEFT             = 0;
const sal_uInt16 AX_POS_TOP                 = 1;
const sal_uInt16 AX_POS_TOPRIGHT            = 2;
const sal_uInt16 AX_POS_LEFT                = 3;
const sal_uInt16 AX_POS_CENTER              = 4;
const sal_uInt16 AX_POS_RIGHT               = 5;
const sal_uInt16 AX_POS_BOTTOMLEFT          = 6;
const sal_uInt16 AX_POS_BOTTOM              = 7;
const sal_uInt16 AX_POS_BOTTOMRIGHT         = 8;

#define AX_PICPOS_IMPL( label, image ) ((AX_POS_##label << 16) | AX_POS_##image)
const sal_uInt32 AX_PICPOS_LEFTTOP          = AX_PICPOS_IMPL( TOPRIGHT,    TOPLEFT );
const sal_uInt32 AX_PICPOS_LEFTCENTER       = AX_PICPOS_IMPL( RIGHT,       LEFT );
const sal_uInt32 AX_PICPOS_LEFTBOTTOM       = AX_PICPOS_IMPL( BOTTOMRIGHT, BOTTOMLEFT );
const sal_uInt32 AX_PICPOS_RIGHTTOP         = AX_PICPOS_IMPL( TOPLEFT,     TOPRIGHT );
const sal_uInt32 AX_PICPOS_RIGHTCENTER      = AX_PICPOS_IMPL( LEFT,        RIGHT );
const sal_uInt32 AX_PICPOS_RIGHTBOTTOM      = AX_PICPOS_IMPL( BOTTOMLEFT,  BOTTOMRIGHT );
const sal_uInt32 AX_PICPOS_ABOVELEFT        = AX_PICPOS_IMPL( BOTTOMLEFT,  TOPLEFT );
const sal_uInt32 AX_PICPOS_ABOVECENTER      = AX_PICPOS_IMPL( BOTTOM,      TOP  );
const sal_uInt32 AX_PICPOS_ABOVERIGHT       = AX_PICPOS_IMPL( BOTTOMRIGHT, TOPRIGHT );
const sal_uInt32 AX_PICPOS_BELOWLEFT        = AX_PICPOS_IMPL( TOPLEFT,     BOTTOMLEFT );
const sal_uInt32 AX_PICPOS_BELOWCENTER      = AX_PICPOS_IMPL( TOP,         BOTTOM );
const sal_uInt32 AX_PICPOS_BELOWRIGHT       = AX_PICPOS_IMPL( TOPRIGHT,    BOTTOMRIGHT );
const sal_uInt32 AX_PICPOS_CENTER           = AX_PICPOS_IMPL( CENTER,      CENTER  );
#undef AX_PICPOS_IMPL

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

const sal_uInt32 AX_TABSTRIP_TABS           = 0;
const sal_uInt32 AX_TABSTRIP_BUTTONS        = 1;
const sal_uInt32 AX_TABSTRIP_NONE           = 2;

const sal_uInt32 AX_CONTAINER_ENABLED       = 0x00000004;
const sal_uInt32 AX_CONTAINER_HASDESIGNEXT  = 0x00004000;
const sal_uInt32 AX_CONTAINER_NOCLASSTABLE  = 0x00008000;

const sal_uInt32 AX_CONTAINER_DEFFLAGS      = 0x00000004;

const sal_Int32 AX_CONTAINER_DEFWIDTH       = 4000;
const sal_Int32 AX_CONTAINER_DEFHEIGHT      = 3000;

const sal_Int32 AX_CONTAINER_CYCLEALL       = 0;
const sal_Int32 AX_CONTAINER_CYCLECURRENT   = 2;

const sal_Int32 AX_CONTAINER_SCR_NONE       = 0x00;
const sal_Int32 AX_CONTAINER_SCR_HOR        = 0x01;
const sal_Int32 AX_CONTAINER_SCR_VER        = 0x02;
const sal_Int32 AX_CONTAINER_SCR_KEEP_HOR   = 0x04;
const sal_Int32 AX_CONTAINER_SCR_KEEP_VER   = 0x08;
const sal_Int32 AX_CONTAINER_SCR_SHOW_LEFT  = 0x10;

// ----------------------------------------------------------------------------

const sal_Int16 API_BORDER_NONE             = 0;
const sal_Int16 API_BORDER_SUNKEN           = 1;
const sal_Int16 API_BORDER_FLAT             = 2;

const sal_Int16 API_STATE_UNCHECKED         = 0;
const sal_Int16 API_STATE_CHECKED           = 1;
const sal_Int16 API_STATE_DONTKNOW          = 2;

} // namespace

// ============================================================================

ControlConverter::ControlConverter( const GraphicHelper& rGraphicHelper, bool bDefaultColorBgr ) :
    mrGraphicHelper( rGraphicHelper ),
    mbDefaultColorBgr( bDefaultColorBgr )
{
}

ControlConverter::~ControlConverter()
{
}

// Generic conversion ---------------------------------------------------------

void ControlConverter::convertPosition( PropertyMap& rPropMap, const AxPairData& rPos ) const
{
    // position is given in 1/100 mm, UNO needs AppFont units
    Point aAppFontPos = mrGraphicHelper.convertHmmToAppFont( Point( rPos.first, rPos.second ) );
    rPropMap.setProperty( PROP_PositionX, aAppFontPos.X );
    rPropMap.setProperty( PROP_PositionY, aAppFontPos.Y );
}

void ControlConverter::convertSize( PropertyMap& rPropMap, const AxPairData& rSize ) const
{
    // size is given in 1/100 mm, UNO needs AppFont units
    Size aAppFontSize = mrGraphicHelper.convertHmmToAppFont( Size( rSize.first, rSize.second ) );
    rPropMap.setProperty( PROP_Width, aAppFontSize.Width );
    rPropMap.setProperty( PROP_Height, aAppFontSize.Height );
}

void ControlConverter::convertColor( PropertyMap& rPropMap, sal_Int32 nPropId, sal_uInt32 nOleColor ) const
{
    rPropMap.setProperty( nPropId, OleHelper::decodeOleColor( mrGraphicHelper, nOleColor, mbDefaultColorBgr ) );
}

void ControlConverter::convertPicture( PropertyMap& rPropMap, const StreamDataSequence& rPicData ) const
{
    if( rPicData.hasElements() )
    {
        OUString aGraphicUrl = mrGraphicHelper.importGraphicObject( rPicData );
        if( aGraphicUrl.getLength() > 0 )
            rPropMap.setProperty( PROP_ImageURL, aGraphicUrl );
    }
}

void ControlConverter::convertOrientation( PropertyMap& rPropMap, bool bHorizontal ) const
{
    namespace AwtScrollBarOrient = ::com::sun::star::awt::ScrollBarOrientation;
    sal_Int32 nScrollOrient = bHorizontal ? AwtScrollBarOrient::HORIZONTAL : AwtScrollBarOrient::VERTICAL;
    rPropMap.setProperty( PROP_Orientation, nScrollOrient );
}

void ControlConverter::convertScrollBar( PropertyMap& rPropMap,
        sal_Int32 nMin, sal_Int32 nMax, sal_Int32 nPosition,
        sal_Int32 nSmallChange, sal_Int32 nLargeChange, bool bAwtModel ) const
{
    rPropMap.setProperty( PROP_ScrollValueMin, ::std::min( nMin, nMax ) );
    rPropMap.setProperty( PROP_ScrollValueMax, ::std::max( nMin, nMax ) );
    rPropMap.setProperty( PROP_LineIncrement, nSmallChange );
    rPropMap.setProperty( PROP_BlockIncrement, nLargeChange );
    rPropMap.setProperty( bAwtModel ? PROP_ScrollValue : PROP_DefaultScrollValue, nPosition );
}

// ActiveX (Forms 2.0) specific conversion ------------------------------------

void ControlConverter::convertAxBackground( PropertyMap& rPropMap,
        sal_uInt32 nBackColor, sal_uInt32 nFlags, ApiTransparencyMode eTranspMode ) const
{
    bool bOpaque = getFlag( nFlags, AX_FLAGS_OPAQUE );
    switch( eTranspMode )
    {
        case API_TRANSPARENCY_NOTSUPPORTED:
            // fake transparency by using system window background if needed
            convertColor( rPropMap, PROP_BackgroundColor, bOpaque ? nBackColor : AX_SYSCOLOR_WINDOWBACK );
        break;
        case API_TRANSPARENCY_PAINTTRANSPARENT:
            rPropMap.setProperty( PROP_PaintTransparent, !bOpaque );
            // run-through intended!
        case API_TRANSPARENCY_VOID:
            // keep transparency by leaving the (void) default property value
            if( bOpaque )
                convertColor( rPropMap, PROP_BackgroundColor, nBackColor );
        break;
    }
}

void ControlConverter::convertAxBorder( PropertyMap& rPropMap,
        sal_uInt32 nBorderColor, sal_Int32 nBorderStyle, sal_Int32 nSpecialEffect ) const
{
    sal_Int16 nBorder = (nBorderStyle == AX_BORDERSTYLE_SINGLE) ? API_BORDER_FLAT :
        ((nSpecialEffect == AX_SPECIALEFFECT_FLAT) ? API_BORDER_NONE : API_BORDER_SUNKEN);
    rPropMap.setProperty( PROP_Border, nBorder );
    convertColor( rPropMap, PROP_BorderColor, nBorderColor );
}

void ControlConverter::convertAxVisualEffect( PropertyMap& rPropMap, sal_Int32 nSpecialEffect ) const
{
    namespace AwtVisualEffect = ::com::sun::star::awt::VisualEffect;
    sal_Int16 nVisualEffect = (nSpecialEffect == AX_SPECIALEFFECT_FLAT) ? AwtVisualEffect::FLAT : AwtVisualEffect::LOOK3D;
    rPropMap.setProperty( PROP_VisualEffect, nVisualEffect );
}

void ControlConverter::convertAxPicture( PropertyMap& rPropMap, const StreamDataSequence& rPicData, sal_uInt32 nPicPos ) const
{
    // the picture
    convertPicture( rPropMap, rPicData );

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
        default:    OSL_ENSURE( false, "ControlConverter::convertAxPicture - unknown picture position" );
    }
    rPropMap.setProperty( PROP_ImagePosition, nImagePos );
}

void ControlConverter::convertAxPicture( PropertyMap& rPropMap, const StreamDataSequence& rPicData,
        sal_Int32 nPicSizeMode, sal_Int32 /*nPicAlign*/, bool /*bPicTiling*/ ) const
{
    // the picture
    convertPicture( rPropMap, rPicData );

    // picture scale mode
    namespace AwtScaleMode = ::com::sun::star::awt::ImageScaleMode;
    sal_Int16 nScaleMode = AwtScaleMode::None;
    switch( nPicSizeMode )
    {
        case AX_PICSIZE_CLIP:       nScaleMode = AwtScaleMode::None;        break;
        case AX_PICSIZE_STRETCH:    nScaleMode = AwtScaleMode::Anisotropic; break;
        case AX_PICSIZE_ZOOM:       nScaleMode = AwtScaleMode::Isotropic;   break;
        default:    OSL_ENSURE( false, "ControlConverter::convertAxPicture - unknown picture size mode" );
    }
    rPropMap.setProperty( PROP_ScaleMode, nScaleMode );
}

void ControlConverter::convertAxState( PropertyMap& rPropMap,
        const OUString& rValue, sal_Int32 nMultiSelect, ApiDefaultStateMode eDefStateMode, bool bAwtModel ) const
{
    bool bBooleanState = eDefStateMode == API_DEFAULTSTATE_BOOLEAN;
    bool bSupportsTriState = eDefStateMode == API_DEFAULTSTATE_TRISTATE;

    // state
    sal_Int16 nState = bSupportsTriState ? API_STATE_DONTKNOW : API_STATE_UNCHECKED;
    if( rValue.getLength() == 1 ) switch( rValue[ 0 ] )
    {
        case '0':   nState = API_STATE_UNCHECKED;   break;
        case '1':   nState = API_STATE_CHECKED;     break;
        // any other string (also empty) means 'dontknow'
    }
    sal_Int32 nPropId = bAwtModel ? PROP_State : PROP_DefaultState;
    if( bBooleanState )
        rPropMap.setProperty( nPropId, nState != API_STATE_UNCHECKED );
    else
        rPropMap.setProperty( nPropId, nState );

    // tristate
    if( bSupportsTriState )
        rPropMap.setProperty( PROP_TriState, nMultiSelect == AX_SELCTION_MULTI );
}

void ControlConverter::convertAxOrientation( PropertyMap& rPropMap,
        const AxPairData& rSize, sal_Int32 nOrientation ) const
{
    bool bHorizontal = true;
    switch( nOrientation )
    {
        case AX_ORIENTATION_AUTO:       bHorizontal = rSize.first > rSize.second;   break;
        case AX_ORIENTATION_VERTICAL:   bHorizontal = false;                        break;
        case AX_ORIENTATION_HORIZONTAL: bHorizontal = true;                         break;
        default:    OSL_ENSURE( false, "ControlConverter::convertAxOrientation - unknown orientation" );
    }
    convertOrientation( rPropMap, bHorizontal );
}

// ============================================================================

ControlModelBase::ControlModelBase() :
    maSize( 0, 0 ),
    mbAwtModel( false )
{
}

ControlModelBase::~ControlModelBase()
{
}

OUString ControlModelBase::getServiceName() const
{
    ApiControlType eCtrlType = getControlType();
    if( mbAwtModel ) switch( eCtrlType )
    {
        case API_CONTROL_BUTTON:        return CREATE_OUSTRING( "com.sun.star.awt.UnoControlButtonModel" );
        case API_CONTROL_FIXEDTEXT:     return CREATE_OUSTRING( "com.sun.star.awt.UnoControlFixedTextModel" );
        case API_CONTROL_IMAGE:         return CREATE_OUSTRING( "com.sun.star.awt.UnoControlImageControlModel" );
        case API_CONTROL_CHECKBOX:      return CREATE_OUSTRING( "com.sun.star.awt.UnoControlCheckBoxModel" );
        case API_CONTROL_RADIOBUTTON:   return CREATE_OUSTRING( "com.sun.star.awt.UnoControlRadioButtonModel" );
        case API_CONTROL_EDIT:          return CREATE_OUSTRING( "com.sun.star.awt.UnoControlEditModel" );
        case API_CONTROL_LISTBOX:       return CREATE_OUSTRING( "com.sun.star.awt.UnoControlListBoxModel" );
        case API_CONTROL_COMBOBOX:      return CREATE_OUSTRING( "com.sun.star.awt.UnoControlComboBoxModel" );
        case API_CONTROL_SPINBUTTON:    return CREATE_OUSTRING( "com.sun.star.awt.UnoControlSpinButtonModel" );
        case API_CONTROL_SCROLLBAR:     return CREATE_OUSTRING( "com.sun.star.awt.UnoControlScrollBarModel" );
        case API_CONTROL_PROGRESSBAR:   return CREATE_OUSTRING( "com.sun.star.awt.UnoControlProgressBarModel" );
        case API_CONTROL_GROUPBOX:      return CREATE_OUSTRING( "com.sun.star.awt.UnoControlGroupBoxModel" );
        case API_CONTROL_DIALOG:        return CREATE_OUSTRING( "com.sun.star.awt.UnoControlDialogModel" );
        default:    OSL_ENSURE( false, "ControlModelBase::getServiceName - no AWT model service supported" );
    }
    else switch( eCtrlType )
    {
        case API_CONTROL_BUTTON:        return CREATE_OUSTRING( "com.sun.star.form.component.CommandButton" );
        case API_CONTROL_FIXEDTEXT:     return CREATE_OUSTRING( "com.sun.star.form.component.FixedText" );
        case API_CONTROL_IMAGE:         return CREATE_OUSTRING( "com.sun.star.form.component.DatabaseImageControl" );
        case API_CONTROL_CHECKBOX:      return CREATE_OUSTRING( "com.sun.star.form.component.CheckBox" );
        case API_CONTROL_RADIOBUTTON:   return CREATE_OUSTRING( "com.sun.star.form.component.RadioButton" );
        case API_CONTROL_EDIT:          return CREATE_OUSTRING( "com.sun.star.form.component.TextField" );
        case API_CONTROL_LISTBOX:       return CREATE_OUSTRING( "com.sun.star.form.component.ListBox" );
        case API_CONTROL_COMBOBOX:      return CREATE_OUSTRING( "com.sun.star.form.component.ComboBox" );
        case API_CONTROL_SPINBUTTON:    return CREATE_OUSTRING( "com.sun.star.form.component.SpinButton" );
        case API_CONTROL_SCROLLBAR:     return CREATE_OUSTRING( "com.sun.star.form.component.ScrollBar" );
        case API_CONTROL_GROUPBOX:      return CREATE_OUSTRING( "com.sun.star.form.component.GroupBox" );
        default:    OSL_ENSURE( false, "ControlModelBase::getServiceName - no form component service supported" );
    }
    return OUString();
}

void ControlModelBase::importProperty( sal_Int32 /*nPropId*/, const OUString& /*rValue*/ )
{
}

void ControlModelBase::importPictureData( sal_Int32 /*nPropId*/, BinaryInputStream& /*rInStrm*/ )
{
}

void ControlModelBase::convertProperties( PropertyMap& /*rPropMap*/, const ControlConverter& /*rConv*/ ) const
{
}

void ControlModelBase::convertSize( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rConv.convertSize( rPropMap, maSize );
}

// ============================================================================

ComCtlModelBase::ComCtlModelBase( sal_uInt32 nDataPartId5, sal_uInt32 nDataPartId6,
        sal_uInt16 nVersion, bool bCommonPart, bool bComplexPart ) :
    maFontData( CREATE_OUSTRING( "Tahoma" ), 82500 ),
    mnFlags( 0 ),
    mnVersion( nVersion ),
    mnDataPartId5( nDataPartId5 ),
    mnDataPartId6( nDataPartId6 ),
    mbCommonPart( bCommonPart ),
    mbComplexPart( bComplexPart )
{
}

bool ComCtlModelBase::importBinaryModel( BinaryInputStream& rInStrm )
{
    // read initial size part and header of the control data part
    if( importSizePart( rInStrm ) && readPartHeader( rInStrm, getDataPartId(), mnVersion ) )
    {
        // if flags part exists, the first int32 of the data part contains its size
        sal_uInt32 nCommonPartSize = mbCommonPart ? rInStrm.readuInt32() : 0;
        // implementations must read the exact amount of data, stream must point to its end afterwards
        importControlData( rInStrm );
        // read following parts
        if( !rInStrm.isEof() &&
            (!mbCommonPart || importCommonPart( rInStrm, nCommonPartSize )) &&
            (!mbComplexPart || importComplexPart( rInStrm )) )
        {
            return !rInStrm.isEof();
        }
    }
    return false;
}

void ComCtlModelBase::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    if( mbCommonPart )
        rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, COMCTL_COMMON_ENABLED ) );
    ControlModelBase::convertProperties( rPropMap, rConv );
}

void ComCtlModelBase::importCommonExtraData( BinaryInputStream& /*rInStrm*/ )
{
}

void ComCtlModelBase::importCommonTrailingData( BinaryInputStream& /*rInStrm*/ )
{
}

sal_uInt32 ComCtlModelBase::getDataPartId() const
{
    switch( mnVersion )
    {
        case 5: return mnDataPartId5;
        case 6: return mnDataPartId6;
    }
    OSL_ENSURE( false, "ComCtlObjectBase::getDataPartId - unxpected version" );
    return SAL_MAX_UINT32;
}

bool ComCtlModelBase::readPartHeader( BinaryInputStream& rInStrm, sal_uInt32 nExpPartId, sal_uInt16 nExpMajor, sal_uInt16 nExpMinor )
{
    // no idea if all this is correct...
    sal_uInt32 nPartId;
    sal_uInt16 nMajor, nMinor;
    rInStrm >> nPartId >> nMinor >> nMajor;
    bool bPartId = nPartId == nExpPartId;
    OSL_ENSURE( bPartId, "ComCtlObjectBase::readPartHeader - unexpected part identifier" );
    bool bVersion = ((nExpMajor == SAL_MAX_UINT16) || (nExpMajor == nMajor)) && ((nExpMinor == SAL_MAX_UINT16) || (nExpMinor == nMinor));
    OSL_ENSURE( bVersion, "ComCtlObjectBase::readPartHeader - unexpected part version" );
    return !rInStrm.isEof() && bPartId && bVersion;
}

bool ComCtlModelBase::importSizePart( BinaryInputStream& rInStrm )
{
    if( readPartHeader( rInStrm, COMCTL_ID_SIZE, 0, 8 ) )
    {
        rInStrm >> maSize.first >> maSize.second;
        return !rInStrm.isEof();
    }
    return false;
}

bool ComCtlModelBase::importCommonPart( BinaryInputStream& rInStrm, sal_uInt32 nPartSize )
{
    sal_Int64 nEndPos = rInStrm.tell() + nPartSize;
    if( (nPartSize >= 16) && readPartHeader( rInStrm, COMCTL_ID_COMMONDATA, 5, 0 ) )
    {
        rInStrm.skip( 4 );
        rInStrm >> mnFlags;
        // implementations may read less than the exact amount of data
        importCommonExtraData( rInStrm );
        rInStrm.seek( nEndPos );
        // implementations must read the exact amount of data, stream must point to its end afterwards
        importCommonTrailingData( rInStrm );
        return !rInStrm.isEof();
    }
    return false;
}

bool ComCtlModelBase::importComplexPart( BinaryInputStream& rInStrm )
{
    if( readPartHeader( rInStrm, COMCTL_ID_COMPLEXDATA, 5, 1 ) )
    {
        sal_uInt32 nContFlags;
        rInStrm >> nContFlags;
        bool bReadOk =
            (!getFlag( nContFlags, COMCTL_COMPLEX_FONT ) || OleHelper::importStdFont( maFontData, rInStrm, true )) &&
            (!getFlag( nContFlags, COMCTL_COMPLEX_MOUSEICON ) || OleHelper::importStdPic( maMouseIcon, rInStrm, true ));
        return bReadOk && !rInStrm.isEof();
    }
    return false;
}

// ============================================================================

ComCtlScrollBarModel::ComCtlScrollBarModel( sal_uInt16 nVersion ) :
    ComCtlModelBase( SAL_MAX_UINT32, COMCTL_ID_SCROLLBAR_60, nVersion, true, true ),
    mnScrollBarFlags( 0x00000011 ),
    mnLargeChange( 1 ),
    mnSmallChange( 1 ),
    mnMin( 0 ),
    mnMax( 32767 ),
    mnPosition( 0 )
{
}

ApiControlType ComCtlScrollBarModel::getControlType() const
{
    return API_CONTROL_SCROLLBAR;
}

void ComCtlScrollBarModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Border, API_BORDER_NONE );
    rConv.convertOrientation( rPropMap, getFlag( mnScrollBarFlags, COMCTL_SCROLLBAR_HOR ) );
    rConv.convertScrollBar( rPropMap, mnMin, mnMax, mnPosition, mnSmallChange, mnLargeChange, mbAwtModel );
    ComCtlModelBase::convertProperties( rPropMap, rConv );
}

void ComCtlScrollBarModel::importControlData( BinaryInputStream& rInStrm )
{
    rInStrm >> mnScrollBarFlags >> mnLargeChange >> mnSmallChange >> mnMin >> mnMax >> mnPosition;
}

// ============================================================================

ComCtlProgressBarModel::ComCtlProgressBarModel( sal_uInt16 nVersion ) :
    ComCtlModelBase( COMCTL_ID_PROGRESSBAR_50, COMCTL_ID_PROGRESSBAR_60, nVersion, true, true ),
    mfMin( 0.0 ),
    mfMax( 100.0 ),
    mnVertical( 0 ),
    mnSmooth( 0 )
{
}

ApiControlType ComCtlProgressBarModel::getControlType() const
{
    return API_CONTROL_PROGRESSBAR;
}

void ComCtlProgressBarModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    sal_uInt16 nBorder = getFlag( mnFlags, COMCTL_COMMON_3DBORDER ) ? API_BORDER_SUNKEN :
        (getFlag( mnFlags, COMCTL_COMMON_FLATBORDER ) ? API_BORDER_FLAT : API_BORDER_NONE);
    rPropMap.setProperty( PROP_Border, nBorder );
    rPropMap.setProperty( PROP_ProgressValueMin, getLimitedValue< sal_Int32, double >( ::std::min( mfMin, mfMax ), 0.0, SAL_MAX_INT32 ) );
    rPropMap.setProperty( PROP_ProgressValueMax, getLimitedValue< sal_Int32, double >( ::std::max( mfMin, mfMax ), 0.0, SAL_MAX_INT32 ) );
    // ComCtl model does not provide current value?
    ComCtlModelBase::convertProperties( rPropMap, rConv );
}

void ComCtlProgressBarModel::importControlData( BinaryInputStream& rInStrm )
{
    rInStrm >> mfMin >> mfMax;
    if( mnVersion == 6 )
        rInStrm >> mnVertical >> mnSmooth;
}

// ============================================================================

AxControlModelBase::AxControlModelBase()
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
                maSize.first = rValue.copy( 0, nSepPos ).toInt32();
                maSize.second = rValue.copy( nSepPos + 1 ).toInt32();
            }
        }
        break;
    }
}

// ============================================================================

AxFontDataModel::AxFontDataModel( bool bSupportsAlign ) :
    mbSupportsAlign( bSupportsAlign )
{
}

void AxFontDataModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_FontName:          maFontData.maFontName = rValue;                                             break;
        case XML_FontEffects:       maFontData.mnFontEffects = AttributeConversion::decodeUnsigned( rValue );   break;
        case XML_FontHeight:        maFontData.mnFontHeight = AttributeConversion::decodeInteger( rValue );     break;
        case XML_FontCharSet:       maFontData.mnFontCharSet = AttributeConversion::decodeInteger( rValue );    break;
        case XML_ParagraphAlign:    maFontData.mnHorAlign = AttributeConversion::decodeInteger( rValue );       break;
        default:                    AxControlModelBase::importProperty( nPropId, rValue );
    }
}

bool AxFontDataModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    return maFontData.importBinaryModel( rInStrm );
}

void AxFontDataModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    namespace cssa = ::com::sun::star::awt;

    // font name
    if( maFontData.maFontName.getLength() > 0 )
        rPropMap.setProperty( PROP_FontName, maFontData.maFontName );

    // font effects
    rPropMap.setProperty( PROP_FontWeight, getFlagValue( maFontData.mnFontEffects, AX_FONTDATA_BOLD, cssa::FontWeight::BOLD, cssa::FontWeight::NORMAL ) );
    rPropMap.setProperty( PROP_FontSlant, getFlagValue< sal_Int16 >( maFontData.mnFontEffects, AX_FONTDATA_ITALIC, cssa::FontSlant_ITALIC, cssa::FontSlant_NONE ) );
    rPropMap.setProperty( PROP_FontUnderline, getFlagValue( maFontData.mnFontEffects, AX_FONTDATA_UNDERLINE, cssa::FontUnderline::SINGLE, cssa::FontUnderline::NONE ) );
    rPropMap.setProperty( PROP_FontStrikeout, getFlagValue( maFontData.mnFontEffects, AX_FONTDATA_STRIKEOUT, cssa::FontStrikeout::SINGLE, cssa::FontStrikeout::NONE ) );
    rPropMap.setProperty( PROP_FontHeight, maFontData.getHeightPoints() );

    // font character set
    rtl_TextEncoding eFontEnc = RTL_TEXTENCODING_DONTKNOW;
    if( (0 <= maFontData.mnFontCharSet) && (maFontData.mnFontCharSet <= SAL_MAX_UINT8) )
        eFontEnc = rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( maFontData.mnFontCharSet ) );
    if( eFontEnc != RTL_TEXTENCODING_DONTKNOW )
        rPropMap.setProperty( PROP_FontCharset, static_cast< sal_Int16 >( eFontEnc ) );

    // text alignment
    if( mbSupportsAlign )
    {
        sal_Int32 nAlign = cssa::TextAlign::LEFT;
        switch( maFontData.mnHorAlign )
        {
            case AX_FONTDATA_LEFT:      nAlign = cssa::TextAlign::LEFT;     break;
            case AX_FONTDATA_RIGHT:     nAlign = cssa::TextAlign::RIGHT;    break;
            case AX_FONTDATA_CENTER:    nAlign = cssa::TextAlign::CENTER;   break;
            default:    OSL_ENSURE( false, "AxFontDataModel::convertProperties - unknown text alignment" );
        }
        // form controls expect short value
        rPropMap.setProperty( PROP_Align, static_cast< sal_Int16 >( nAlign ) );
    }

    // process base class properties
    AxControlModelBase::convertProperties( rPropMap, rConv );
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
        case XML_Caption:               maCaption = rValue;                                                 break;
        case XML_ForeColor:             mnTextColor = AttributeConversion::decodeUnsigned( rValue );        break;
        case XML_BackColor:             mnBackColor = AttributeConversion::decodeUnsigned( rValue );        break;
        case XML_VariousPropertyBits:   mnFlags = AttributeConversion::decodeUnsigned( rValue );            break;
        case XML_PicturePosition:       mnPicturePos = AttributeConversion::decodeUnsigned( rValue );       break;
        case XML_TakeFocusOnClick:      mbFocusOnClick = AttributeConversion::decodeInteger( rValue ) != 0; break;
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

bool AxCommandButtonModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readIntProperty< sal_uInt32 >( mnTextColor );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readStringProperty( maCaption );
    aReader.readIntProperty< sal_uInt32 >( mnPicturePos );
    aReader.readPairProperty( maSize );
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.readPictureProperty( maPictureData );
    aReader.skipIntProperty< sal_uInt16 >(); // accelerator
    aReader.readBoolProperty( mbFocusOnClick, true ); // binary flag means "do not take focus"
    aReader.skipPictureProperty(); // mouse icon
    return aReader.finalizeImport() && AxFontDataModel::importBinaryModel( rInStrm );
}

ApiControlType AxCommandButtonModel::getControlType() const
{
    return API_CONTROL_BUTTON;
}

void AxCommandButtonModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_FocusOnClick, mbFocusOnClick );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_MIDDLE );
    rConv.convertColor( rPropMap, PROP_TextColor, mnTextColor );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    rConv.convertAxPicture( rPropMap, maPictureData, mnPicturePos );
    AxFontDataModel::convertProperties( rPropMap, rConv );
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
        case XML_ForeColor:             mnTextColor = AttributeConversion::decodeUnsigned( rValue );    break;
        case XML_BackColor:             mnBackColor = AttributeConversion::decodeUnsigned( rValue );    break;
        case XML_VariousPropertyBits:   mnFlags = AttributeConversion::decodeUnsigned( rValue );        break;
        case XML_BorderColor:           mnBorderColor = AttributeConversion::decodeUnsigned( rValue );  break;
        case XML_BorderStyle:           mnBorderStyle = AttributeConversion::decodeInteger( rValue );   break;
        case XML_SpecialEffect:         mnSpecialEffect = AttributeConversion::decodeInteger( rValue ); break;
        default:                        AxFontDataModel::importProperty( nPropId, rValue );
    }
}

bool AxLabelModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readIntProperty< sal_uInt32 >( mnTextColor );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readStringProperty( maCaption );
    aReader.skipIntProperty< sal_uInt32 >(); // picture position
    aReader.readPairProperty( maSize );
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.readIntProperty< sal_uInt32 >( mnBorderColor );
    aReader.readIntProperty< sal_uInt16 >( mnBorderStyle );
    aReader.readIntProperty< sal_uInt16 >( mnSpecialEffect );
    aReader.skipPictureProperty(); // picture
    aReader.skipIntProperty< sal_uInt16 >(); // accelerator
    aReader.skipPictureProperty(); // mouse icon
    return aReader.finalizeImport() && AxFontDataModel::importBinaryModel( rInStrm );
}

ApiControlType AxLabelModel::getControlType() const
{
    return API_CONTROL_FIXEDTEXT;
}

void AxLabelModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_TOP );
    rConv.convertColor( rPropMap, PROP_TextColor, mnTextColor );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxBorder( rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxFontDataModel::convertProperties( rPropMap, rConv );
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
        case XML_BackColor:             mnBackColor = AttributeConversion::decodeUnsigned( rValue );      break;
        case XML_VariousPropertyBits:   mnFlags = AttributeConversion::decodeUnsigned( rValue );          break;
        case XML_BorderColor:           mnBorderColor = AttributeConversion::decodeUnsigned( rValue );    break;
        case XML_BorderStyle:           mnBorderStyle = AttributeConversion::decodeInteger( rValue );     break;
        case XML_SpecialEffect:         mnSpecialEffect = AttributeConversion::decodeInteger( rValue );   break;
        case XML_SizeMode:              mnPicSizeMode = AttributeConversion::decodeInteger( rValue );     break;
        case XML_PictureAlignment:      mnPicAlign = AttributeConversion::decodeInteger( rValue );        break;
        case XML_PictureTiling:         mbPicTiling = AttributeConversion::decodeInteger( rValue ) != 0;  break;
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

bool AxImageModel::importBinaryModel( BinaryInputStream& rInStrm )
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
    aReader.readPairProperty( maSize );
    aReader.readPictureProperty( maPictureData );
    aReader.readIntProperty< sal_uInt8 >( mnPicAlign );
    aReader.readBoolProperty( mbPicTiling );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.skipPictureProperty(); // mouse icon
    return aReader.finalizeImport();
}

ApiControlType AxImageModel::getControlType() const
{
    return API_CONTROL_IMAGE;
}

void AxImageModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxBorder( rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    rConv.convertAxPicture( rPropMap, maPictureData, mnPicSizeMode, mnPicAlign, mbPicTiling );
    AxControlModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxMorphDataModelBase::AxMorphDataModelBase() :
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

void AxMorphDataModelBase::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_Caption:               maCaption = rValue;                                             break;
        case XML_Value:                 maValue = rValue;                                               break;
        case XML_GroupName:             maGroupName = rValue;                                           break;
        case XML_ForeColor:             mnTextColor = AttributeConversion::decodeUnsigned( rValue );    break;
        case XML_BackColor:             mnBackColor = AttributeConversion::decodeUnsigned( rValue );    break;
        case XML_VariousPropertyBits:   mnFlags = AttributeConversion::decodeUnsigned( rValue );        break;
        case XML_PicturePosition:       mnPicturePos = AttributeConversion::decodeUnsigned( rValue );   break;
        case XML_BorderColor:           mnBorderColor = AttributeConversion::decodeUnsigned( rValue );  break;
        case XML_BorderStyle:           mnBorderStyle = AttributeConversion::decodeInteger( rValue );   break;
        case XML_SpecialEffect:         mnSpecialEffect = AttributeConversion::decodeInteger( rValue ); break;
        case XML_DisplayStyle:          mnDisplayStyle = AttributeConversion::decodeInteger( rValue );  break;
        case XML_MultiSelect:           mnMultiSelect = AttributeConversion::decodeInteger( rValue );   break;
        case XML_ScrollBars:            mnScrollBars = AttributeConversion::decodeInteger( rValue );    break;
        case XML_MatchEntry:            mnMatchEntry = AttributeConversion::decodeInteger( rValue );    break;
        case XML_ShowDropButtonWhen:    mnShowDropButton = AttributeConversion::decodeInteger( rValue );break;
        case XML_MaxLength:             mnMaxLength = AttributeConversion::decodeInteger( rValue );     break;
        case XML_PasswordChar:          mnPasswordChar = AttributeConversion::decodeInteger( rValue );  break;
        case XML_ListRows:              mnListRows = AttributeConversion::decodeInteger( rValue );      break;
        default:                        AxFontDataModel::importProperty( nPropId, rValue );
    }
}

void AxMorphDataModelBase::importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm )
{
    switch( nPropId )
    {
        case XML_Picture:   OleHelper::importStdPic( maPictureData, rInStrm, true );    break;
        default:            AxFontDataModel::importPictureData( nPropId, rInStrm );
    }
}

bool AxMorphDataModelBase::importBinaryModel( BinaryInputStream& rInStrm )
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
    aReader.readPairProperty( maSize );
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
    return aReader.finalizeImport() && AxFontDataModel::importBinaryModel( rInStrm );
}

void AxMorphDataModelBase::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rConv.convertColor( rPropMap, PROP_TextColor, mnTextColor );
    AxFontDataModel::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxToggleButtonModel::AxToggleButtonModel()
{
}

ApiControlType AxToggleButtonModel::getControlType() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_TOGGLE, "AxToggleButtonModel::getControlType - invalid control type" );
    return API_CONTROL_BUTTON;
}

void AxToggleButtonModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_MIDDLE );
    rPropMap.setProperty( PROP_Toggle, true );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    rConv.convertAxPicture( rPropMap, maPictureData, mnPicturePos );
    rConv.convertAxState( rPropMap, maValue, mnMultiSelect, API_DEFAULTSTATE_BOOLEAN, mbAwtModel );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxCheckBoxModel::AxCheckBoxModel()
{
}

ApiControlType AxCheckBoxModel::getControlType() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_CHECKBOX, "AxCheckBoxModel::getControlType - invalid control type" );
    return API_CONTROL_CHECKBOX;
}

void AxCheckBoxModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_MIDDLE );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxVisualEffect( rPropMap, mnSpecialEffect );
    rConv.convertAxPicture( rPropMap, maPictureData, mnPicturePos );
    rConv.convertAxState( rPropMap, maValue, mnMultiSelect, API_DEFAULTSTATE_TRISTATE, mbAwtModel );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxOptionButtonModel::AxOptionButtonModel()
{
}

ApiControlType AxOptionButtonModel::getControlType() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_OPTBUTTON, "AxOptionButtonModel::getControlType - invalid control type" );
    return API_CONTROL_RADIOBUTTON;
}

void AxOptionButtonModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_VerticalAlign, ::com::sun::star::style::VerticalAlignment_MIDDLE );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxVisualEffect( rPropMap, mnSpecialEffect );
    rConv.convertAxPicture( rPropMap, maPictureData, mnPicturePos );
    rConv.convertAxState( rPropMap, maValue, mnMultiSelect, API_DEFAULTSTATE_SHORT, mbAwtModel );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxTextBoxModel::AxTextBoxModel()
{
}

ApiControlType AxTextBoxModel::getControlType() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_TEXT, "AxTextBoxModel::getControlType - invalid control type" );
    return API_CONTROL_EDIT;
}

void AxTextBoxModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_MULTILINE ) );
    rPropMap.setProperty( PROP_HideInactiveSelection, getFlag( mnFlags, AX_FLAGS_HIDESELECTION ) );
    rPropMap.setProperty( mbAwtModel ? PROP_Text : PROP_DefaultText, maValue );
    rPropMap.setProperty( PROP_MaxTextLen, getLimitedValue< sal_Int16, sal_Int32 >( mnMaxLength, 0, SAL_MAX_INT16 ) );
    if( (0 < mnPasswordChar) && (mnPasswordChar <= SAL_MAX_INT16) )
        rPropMap.setProperty( PROP_EchoChar, static_cast< sal_Int16 >( mnPasswordChar ) );
    rPropMap.setProperty( PROP_HScroll, getFlag( mnScrollBars, AX_SCROLLBAR_HORIZONTAL ) );
    rPropMap.setProperty( PROP_VScroll, getFlag( mnScrollBars, AX_SCROLLBAR_VERTICAL ) );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxBorder( rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxListBoxModel::AxListBoxModel()
{
}

ApiControlType AxListBoxModel::getControlType() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_LISTBOX, "AxListBoxModel::getControlType - invalid control type" );
    return API_CONTROL_LISTBOX;
}

void AxListBoxModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    bool bMultiSelect = (mnMultiSelect == AX_SELCTION_MULTI) || (mnMultiSelect == AX_SELCTION_EXTENDED);
    rPropMap.setProperty( PROP_MultiSelection, bMultiSelect );
    rPropMap.setProperty( PROP_Dropdown, false );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxBorder( rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxComboBoxModel::AxComboBoxModel()
{
}

ApiControlType AxComboBoxModel::getControlType() const
{
    OSL_ENSURE( (mnDisplayStyle == AX_DISPLAYSTYLE_COMBOBOX) || (mnDisplayStyle == AX_DISPLAYSTYLE_DROPDOWN), "AxComboBoxModel::getControlType - invalid control type" );
    return (mnDisplayStyle == AX_DISPLAYSTYLE_DROPDOWN) ? API_CONTROL_LISTBOX : API_CONTROL_COMBOBOX;
}

void AxComboBoxModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    if( mnDisplayStyle != AX_DISPLAYSTYLE_DROPDOWN )
    {
        rPropMap.setProperty( PROP_HideInactiveSelection, getFlag( mnFlags, AX_FLAGS_HIDESELECTION ) );
        rPropMap.setProperty( mbAwtModel ? PROP_Text : PROP_DefaultText, maValue );
        rPropMap.setProperty( PROP_MaxTextLen, getLimitedValue< sal_Int16, sal_Int32 >( mnMaxLength, 0, SAL_MAX_INT16 ) );
        bool bAutoComplete = (mnMatchEntry == AX_MATCHENTRY_FIRSTLETTER) || (mnMatchEntry == AX_MATCHENTRY_COMPLETE);
        rPropMap.setProperty( PROP_Autocomplete, bAutoComplete );
    }
    bool bShowDropdown = (mnShowDropButton == AX_SHOWDROPBUTTON_FOCUS) || (mnShowDropButton == AX_SHOWDROPBUTTON_ALWAYS);
    rPropMap.setProperty( PROP_Dropdown, bShowDropdown );
    rPropMap.setProperty( PROP_LineCount, getLimitedValue< sal_Int16, sal_Int32 >( mnListRows, 1, SAL_MAX_INT16 ) );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxBorder( rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
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

ApiControlType AxSpinButtonModel::getControlType() const
{
    return API_CONTROL_SPINBUTTON;
}

void AxSpinButtonModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_ForeColor:             mnArrowColor = AttributeConversion::decodeUnsigned( rValue );   break;
        case XML_BackColor:             mnBackColor = AttributeConversion::decodeUnsigned( rValue );    break;
        case XML_VariousPropertyBits:   mnFlags = AttributeConversion::decodeUnsigned( rValue );        break;
        case XML_Orientation:           mnOrientation = AttributeConversion::decodeInteger( rValue );   break;
        case XML_Min:                   mnMin = AttributeConversion::decodeInteger( rValue );           break;
        case XML_Max:                   mnMax = AttributeConversion::decodeInteger( rValue );           break;
        case XML_Position:              mnPosition = AttributeConversion::decodeInteger( rValue );      break;
        case XML_SmallChange:           mnSmallChange = AttributeConversion::decodeInteger( rValue );   break;
        case XML_Delay:                 mnDelay = AttributeConversion::decodeInteger( rValue );         break;
        default:                        AxControlModelBase::importProperty( nPropId, rValue );
    }
}

bool AxSpinButtonModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readIntProperty< sal_uInt32 >( mnArrowColor );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readPairProperty( maSize );
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
    return aReader.finalizeImport();
}

void AxSpinButtonModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    sal_Int32 nMin = ::std::min( mnMin, mnMax );
    sal_Int32 nMax = ::std::max( mnMin, mnMax );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rPropMap.setProperty( PROP_SpinValueMin, nMin );
    rPropMap.setProperty( PROP_SpinValueMax, nMax );
    rPropMap.setProperty( PROP_SpinIncrement, mnSmallChange );
    rPropMap.setProperty( mbAwtModel ? PROP_SpinValue : PROP_DefaultSpinValue, mnPosition );
    rPropMap.setProperty( PROP_Repeat, true );
    rPropMap.setProperty( PROP_RepeatDelay, mnDelay );
    rPropMap.setProperty( PROP_Border, API_BORDER_NONE );
    rConv.convertColor( rPropMap, PROP_SymbolColor, mnArrowColor );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    rConv.convertAxOrientation( rPropMap, maSize, mnOrientation );
    AxControlModelBase::convertProperties( rPropMap, rConv );
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

ApiControlType AxScrollBarModel::getControlType() const
{
    return API_CONTROL_SCROLLBAR;
}

void AxScrollBarModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_ForeColor:             mnArrowColor = AttributeConversion::decodeUnsigned( rValue );   break;
        case XML_BackColor:             mnBackColor = AttributeConversion::decodeUnsigned( rValue );    break;
        case XML_VariousPropertyBits:   mnFlags = AttributeConversion::decodeUnsigned( rValue );        break;
        case XML_Orientation:           mnOrientation = AttributeConversion::decodeInteger( rValue );   break;
        case XML_ProportionalThumb:     mnPropThumb = AttributeConversion::decodeInteger( rValue );     break;
        case XML_Min:                   mnMin = AttributeConversion::decodeInteger( rValue );           break;
        case XML_Max:                   mnMax = AttributeConversion::decodeInteger( rValue );           break;
        case XML_Position:              mnPosition = AttributeConversion::decodeInteger( rValue );      break;
        case XML_SmallChange:           mnSmallChange = AttributeConversion::decodeInteger( rValue );   break;
        case XML_LargeChange:           mnLargeChange = AttributeConversion::decodeInteger( rValue );   break;
        case XML_Delay:                 mnDelay = AttributeConversion::decodeInteger( rValue );         break;
        default:                        AxControlModelBase::importProperty( nPropId, rValue );
    }
}

bool AxScrollBarModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readIntProperty< sal_uInt32 >( mnArrowColor );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readPairProperty( maSize );
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
    return aReader.finalizeImport();
}

void AxScrollBarModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_FLAGS_ENABLED ) );
    rPropMap.setProperty( PROP_RepeatDelay, mnDelay );
    rPropMap.setProperty( PROP_Border, API_BORDER_NONE );
    if( (mnPropThumb == AX_PROPTHUMB_ON) && (mnMin != mnMax) && (mnLargeChange > 0) )
    {
        // use double to prevent integer overflow in division (fInterval+mnLargeChange may become 0 when performed as int)
        double fInterval = fabs( static_cast< double >( mnMax - mnMin ) );
        sal_Int32 nThumbLen = getLimitedValue< sal_Int32, double >( (fInterval * mnLargeChange) / (fInterval + mnLargeChange), 1, SAL_MAX_INT32 );
        rPropMap.setProperty( PROP_VisibleSize, nThumbLen );
    }
    rConv.convertColor( rPropMap, PROP_SymbolColor, mnArrowColor );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    rConv.convertAxOrientation( rPropMap, maSize, mnOrientation );
    rConv.convertScrollBar( rPropMap, mnMin, mnMax, mnPosition, mnSmallChange, mnLargeChange, mbAwtModel );
    AxControlModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxTabStripModel::AxTabStripModel() :
    AxFontDataModel( false ),   // no support for Align property
    mnBackColor( AX_SYSCOLOR_BUTTONFACE ),
    mnTextColor( AX_SYSCOLOR_BUTTONTEXT ),
    mnFlags( AX_TABSTRIP_DEFFLAGS ),
    mnSelectedTab( -1 ),
    mnTabStyle( AX_TABSTRIP_TABS ),
    mnTabFlagCount( 0 )
{
}

bool AxTabStripModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readIntProperty< sal_Int32 >( mnSelectedTab );
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnTextColor );
    aReader.skipUndefinedProperty();
    aReader.readPairProperty( maSize );
    aReader.readStringArrayProperty( maCaptions );
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.skipUndefinedProperty();
    aReader.skipIntProperty< sal_uInt32 >(); // tab orientation
    aReader.readIntProperty< sal_uInt32 >( mnTabStyle );
    aReader.skipBoolProperty(); // multiple rows
    aReader.skipIntProperty< sal_uInt32 >(); // fixed width
    aReader.skipIntProperty< sal_uInt32 >(); // fixed height
    aReader.skipBoolProperty(); // tooltips
    aReader.skipUndefinedProperty();
    aReader.skipStringArrayProperty(); // tooltip strings
    aReader.skipUndefinedProperty();
    aReader.skipStringArrayProperty(); // tab names
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.skipBoolProperty(); // new version
    aReader.skipIntProperty< sal_uInt32 >(); // tabs allocated
    aReader.skipStringArrayProperty(); // tags
    aReader.readIntProperty< sal_uInt32 >( mnTabFlagCount );
    aReader.skipStringArrayProperty(); // accelerators
    aReader.skipPictureProperty(); // mouse icon
    return aReader.finalizeImport() && AxFontDataModel::importBinaryModel( rInStrm );
}

ApiControlType AxTabStripModel::getControlType() const
{
    return API_CONTROL_TABSTRIP;
}

void AxTabStripModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Decoration, mnTabStyle != AX_TABSTRIP_NONE );
    rPropMap.setProperty( PROP_MultiPageValue, mnSelectedTab );
    rConv.convertColor( rPropMap, PROP_BackgroundColor, mnBackColor );
    AxFontDataModel::convertProperties( rPropMap, rConv );
}

OUString AxTabStripModel::getCaption( sal_Int32 nIndex ) const
{
    return ContainerHelper::getVectorElement( maCaptions, nIndex, OUString() );
}

// ============================================================================

AxContainerModelBase::AxContainerModelBase( bool bFontSupport ) :
    AxFontDataModel( false ),   // no support for Align property
    maLogicalSize( AX_CONTAINER_DEFWIDTH, AX_CONTAINER_DEFHEIGHT ),
    maScrollPos( 0, 0 ),
    mnBackColor( AX_SYSCOLOR_BUTTONFACE ),
    mnTextColor( AX_SYSCOLOR_BUTTONTEXT ),
    mnFlags( AX_CONTAINER_DEFFLAGS ),
    mnBorderColor( AX_SYSCOLOR_BUTTONTEXT ),
    mnBorderStyle( AX_BORDERSTYLE_NONE ),
    mnScrollBars( AX_CONTAINER_SCR_NONE ),
    mnCycleType( AX_CONTAINER_CYCLEALL ),
    mnSpecialEffect( AX_SPECIALEFFECT_FLAT ),
    mnPicAlign( AX_PICALIGN_CENTER ),
    mnPicSizeMode( AX_PICSIZE_CLIP ),
    mbPicTiling( false ),
    mbFontSupport( bFontSupport )
{
    setAwtModelMode();
    // different default size for frame
    maSize = AxPairData( AX_CONTAINER_DEFWIDTH, AX_CONTAINER_DEFHEIGHT );
}

void AxContainerModelBase::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    if( nPropId == XML_Caption )
        maCaption = rValue;
}

bool AxContainerModelBase::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.skipUndefinedProperty();
    aReader.readIntProperty< sal_uInt32 >( mnBackColor );
    aReader.readIntProperty< sal_uInt32 >( mnTextColor );
    aReader.skipIntProperty< sal_uInt32 >(); // next availbale control ID
    aReader.skipUndefinedProperty();
    aReader.skipUndefinedProperty();
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readIntProperty< sal_uInt8 >( mnBorderStyle );
    aReader.skipIntProperty< sal_uInt8 >(); // mouse pointer
    aReader.readIntProperty< sal_uInt8 >( mnScrollBars );
    aReader.readPairProperty( maSize );
    aReader.readPairProperty( maLogicalSize );
    aReader.readPairProperty( maScrollPos );
    aReader.skipIntProperty< sal_uInt32 >(); // number of control groups
    aReader.skipUndefinedProperty();
    aReader.skipPictureProperty(); // mouse icon
    aReader.readIntProperty< sal_uInt8 >( mnCycleType );
    aReader.readIntProperty< sal_uInt8 >( mnSpecialEffect );
    aReader.readIntProperty< sal_uInt32 >( mnBorderColor );
    aReader.readStringProperty( maCaption );
    aReader.readFontProperty( maFontData );
    aReader.readPictureProperty( maPictureData );
    aReader.skipIntProperty< sal_Int32 >(); // zoom
    aReader.readIntProperty< sal_uInt8 >( mnPicAlign );
    aReader.readBoolProperty( mbPicTiling );
    aReader.readIntProperty< sal_uInt8 >( mnPicSizeMode );
    aReader.skipIntProperty< sal_uInt32 >(); // shape cookie
    aReader.skipIntProperty< sal_uInt32 >(); // draw buffer size
    return aReader.finalizeImport();
}

void AxContainerModelBase::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    if( mbFontSupport )
    {
        rConv.convertColor( rPropMap, PROP_TextColor, mnTextColor );
        AxFontDataModel::convertProperties( rPropMap, rConv );
    }
}

bool AxContainerModelBase::importClassTable( BinaryInputStream& rInStrm, AxClassTable& orClassTable )
{
    bool bValid = true;
    orClassTable.clear();
    if( !getFlag( mnFlags, AX_CONTAINER_NOCLASSTABLE ) )
    {
        sal_uInt16 nCount = rInStrm.readuInt16();
        for( sal_uInt16 nIndex = 0; bValid && (nIndex < nCount); ++nIndex )
        {
            orClassTable.push_back( OUString() );
            AxBinaryPropertyReader aReader( rInStrm );
            aReader.readGuidProperty( orClassTable.back() );
            aReader.skipGuidProperty(); // source interface GUID
            aReader.skipUndefinedProperty();
            aReader.skipGuidProperty(); // default interface GUID
            aReader.skipIntProperty< sal_uInt32 >(); // class table and var flags
            aReader.skipIntProperty< sal_uInt32 >(); // method count
            aReader.skipIntProperty< sal_Int32 >(); // IDispatch identifier for linked cell access
            aReader.skipIntProperty< sal_uInt16 >(); // get function index for linked cell access
            aReader.skipIntProperty< sal_uInt16 >(); // put function index for linked cell access
            aReader.skipIntProperty< sal_uInt16 >(); // linked cell access property type
            aReader.skipIntProperty< sal_uInt16 >(); // get function index of value
            aReader.skipIntProperty< sal_uInt16 >(); // put function index of value
            aReader.skipIntProperty< sal_uInt16 >(); // value type
            aReader.skipIntProperty< sal_Int32 >(); // IDispatch identifier for source range access
            aReader.skipIntProperty< sal_uInt16 >(); // get function index for source range access
            bValid = aReader.finalizeImport();
        }
    }
    return bValid;
}

// ============================================================================

AxFrameModel::AxFrameModel() :
    AxContainerModelBase( true )
{
}

ApiControlType AxFrameModel::getControlType() const
{
    return API_CONTROL_GROUPBOX;
}

void AxFrameModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_CONTAINER_ENABLED ) );
    AxContainerModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxFormPageModel::AxFormPageModel()
{
}

ApiControlType AxFormPageModel::getControlType() const
{
    return API_CONTROL_PAGE;
}

void AxFormPageModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Title, maCaption );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_CONTAINER_ENABLED ) );
    rConv.convertColor( rPropMap, PROP_BackgroundColor, mnBackColor );
    AxContainerModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

AxMultiPageModel::AxMultiPageModel()
{
}

ApiControlType AxMultiPageModel::getControlType() const
{
    return API_CONTROL_MULTIPAGE;
}

void AxMultiPageModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_CONTAINER_ENABLED ) );
    if( mxTabStrip.get() )
        mxTabStrip->convertProperties( rPropMap, rConv );
    AxContainerModelBase::convertProperties( rPropMap, rConv );
}

void AxMultiPageModel::setTabStripModel( const AxTabStripModelRef& rxTabStrip )
{
    mxTabStrip = rxTabStrip;
}

// ============================================================================

AxUserFormModel::AxUserFormModel()
{
}

ApiControlType AxUserFormModel::getControlType() const
{
    return API_CONTROL_DIALOG;
}

void AxUserFormModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Title, maCaption );
    rConv.convertColor( rPropMap, PROP_BackgroundColor, mnBackColor );
    AxContainerModelBase::convertProperties( rPropMap, rConv );
}

// ============================================================================

EmbeddedControl::EmbeddedControl( const OUString& rName ) :
    maName( rName )
{
}

EmbeddedControl::~EmbeddedControl()
{
}

ControlModelRef EmbeddedControl::createModel( const OUString& rClassId )
{
    OUString aClassId = rClassId.toAsciiUpperCase();
    if( aClassId.equalsAscii( AX_GUID_COMMANDBUTTON ) )
        mxModel.reset( new AxCommandButtonModel );
    else if( aClassId.equalsAscii( AX_GUID_LABEL ) )
        mxModel.reset( new AxLabelModel );
    else if( aClassId.equalsAscii( AX_GUID_IMAGE ) )
        mxModel.reset( new AxImageModel );
    else if( aClassId.equalsAscii( AX_GUID_TOGGLEBUTTON ) )
        mxModel.reset( new AxToggleButtonModel );
    else if( aClassId.equalsAscii( AX_GUID_CHECKBOX ) )
        mxModel.reset( new AxCheckBoxModel );
    else if( aClassId.equalsAscii( AX_GUID_OPTIONBUTTON ) )
        mxModel.reset( new AxOptionButtonModel );
    else if( aClassId.equalsAscii( AX_GUID_TEXTBOX ) )
        mxModel.reset( new AxTextBoxModel );
    else if( aClassId.equalsAscii( AX_GUID_LISTBOX ) )
        mxModel.reset( new AxListBoxModel );
    else if( aClassId.equalsAscii( AX_GUID_COMBOBOX ) )
        mxModel.reset( new AxComboBoxModel );
    else if( aClassId.equalsAscii( AX_GUID_SPINBUTTON ) )
        mxModel.reset( new AxSpinButtonModel );
    else if( aClassId.equalsAscii( AX_GUID_SCROLLBAR ) )
        mxModel.reset( new AxScrollBarModel );
    else if( aClassId.equalsAscii( AX_GUID_FRAME ) )
        mxModel.reset( new AxFrameModel );
    else if( aClassId.equalsAscii( COMCTL_GUID_SCROLLBAR_60 ) )
        mxModel.reset( new ComCtlScrollBarModel( 6 ) );
    else
        mxModel.reset();

    // embedded controls are form component instances
    if( mxModel.get() )
        mxModel->setFormComponentMode();
    
    return mxModel;
}

OUString EmbeddedControl::getServiceName() const
{
    return mxModel.get() ? mxModel->getServiceName() : OUString();
}

bool EmbeddedControl::convertProperties( const Reference< XControlModel >& rxCtrlModel, const ControlConverter& rConv ) const
{
    if( mxModel.get() && rxCtrlModel.is() && (maName.getLength() > 0) )
    {
        PropertyMap aPropMap;
        aPropMap.setProperty( PROP_Name, maName );
        mxModel->convertProperties( aPropMap, rConv );
        PropertySet aPropSet( rxCtrlModel );
        aPropSet.setProperties( aPropMap );
        return true;
    }
    return false;
}

// ============================================================================

EmbeddedForm::EmbeddedForm( const Reference< XMultiServiceFactory >& rxModelFactory,
        const Reference< XDrawPage >& rxDrawPage, const GraphicHelper& rGraphicHelper, bool bDefaultColorBgr ) :
    ControlConverter( rGraphicHelper, bDefaultColorBgr ),
    mxModelFactory( rxModelFactory ),
    mxFormsSupp( rxDrawPage, UNO_QUERY )
{
    OSL_ENSURE( mxModelFactory.is(), "EmbeddedForm::EmbeddedForm - missing service factory" );
}

Reference< XControlModel > EmbeddedForm::convertAndInsert( const EmbeddedControl& rControl )
{
    if( mxModelFactory.is() && rControl.hasModel() ) try
    {
        // create the UNO control model
        OUString aServiceName = rControl.getServiceName();
        Reference< XFormComponent > xFormComp( mxModelFactory->createInstance( aServiceName ), UNO_QUERY_THROW );
        Reference< XControlModel > xCtrlModel( xFormComp, UNO_QUERY_THROW );

        // insert the control into the form
        Reference< XIndexContainer > xFormIC( createForm(), UNO_SET_THROW );
        sal_Int32 nNewIndex = xFormIC->getCount();
        xFormIC->insertByIndex( nNewIndex, Any( xFormComp ) );
        
        // convert the control properties
        if( rControl.convertProperties( xCtrlModel, *this ) )
            return xCtrlModel;
    }
    catch( Exception& )
    {
    }
    return Reference< XControlModel >();
}

Reference< XIndexContainer > EmbeddedForm::createForm()
{
    if( mxFormsSupp.is() )
    {
        try
        {
            Reference< XNameContainer > xFormsNC( mxFormsSupp->getForms(), UNO_SET_THROW );
            OUString aFormName = CREATE_OUSTRING( "Standard" );
            if( xFormsNC->hasByName( aFormName ) )
            {
                mxFormIC.set( xFormsNC->getByName( aFormName ), UNO_QUERY_THROW );
            }
            else if( mxModelFactory.is() )
            {
                Reference< XForm > xForm( mxModelFactory->createInstance( CREATE_OUSTRING( "com.sun.star.form.component.Form" ) ), UNO_QUERY_THROW );
                xFormsNC->insertByName( aFormName, Any( xForm ) );
                mxFormIC.set( xForm, UNO_QUERY_THROW );
            }
        }
        catch( Exception& )
        {
        }
        // always clear the forms supplier to not try to create the form again
        mxFormsSupp.clear();
    }
    return mxFormIC;
}

// ============================================================================

} // namespace ole
} // namespace oox
