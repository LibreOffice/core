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

#include "oox/ole/axcontrol.hxx"

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
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <comphelper/string.hxx>
#include <rtl/tencinfo.h>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/ole/axbinarywriter.hxx"
#include "tools/string.hxx"
namespace oox {
namespace ole {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::form::binding;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

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

const sal_Int32 AX_MATCHENTRY_FIRSTLETTER   = 0;
const sal_Int32 AX_MATCHENTRY_COMPLETE      = 1;
const sal_Int32 AX_MATCHENTRY_NONE          = 2;

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

// ----------------------------------------------------------------------------

/** Tries to extract a range address from a defined name. */
bool lclExtractRangeFromName( CellRangeAddress& orRangeAddr, const Reference< XModel >& rxDocModel, const OUString& rAddressString )
{
    try
    {
        PropertySet aPropSet( rxDocModel );
        Reference< XNameAccess > xRangesNA( aPropSet.getAnyProperty( PROP_NamedRanges ), UNO_QUERY_THROW );
        Reference< XCellRangeReferrer > xReferrer( xRangesNA->getByName( rAddressString ), UNO_QUERY_THROW );
        Reference< XCellRangeAddressable > xAddressable( xReferrer->getReferredCells(), UNO_QUERY_THROW );
        orRangeAddr = xAddressable->getRangeAddress();
        return true;
    }
    catch(const Exception& )
    {
    }
    return false;
}

bool lclExtractAddressFromName( CellAddress& orAddress, const Reference< XModel >& rxDocModel, const OUString& rAddressString )
{
    CellRangeAddress aRangeAddr;
    if( lclExtractRangeFromName( aRangeAddr, rxDocModel, rAddressString ) &&
        (aRangeAddr.StartColumn == aRangeAddr.EndColumn) &&
        (aRangeAddr.StartRow == aRangeAddr.EndRow) )
    {
        orAddress.Sheet = aRangeAddr.Sheet;
        orAddress.Column = aRangeAddr.StartColumn;
        orAddress.Row = aRangeAddr.StartRow;
        return true;
    }
    return false;
}

void lclPrepareConverter( PropertySet& rConverter, const Reference< XModel >& rxDocModel,
        const OUString& rAddressString, sal_Int32 nRefSheet, bool bRange )
{
    if( !rConverter.is() ) try
    {
        Reference< XMultiServiceFactory > xModelFactory( rxDocModel, UNO_QUERY_THROW );
        OUString aServiceName = bRange ?
            CREATE_OUSTRING( "com.sun.star.table.CellRangeAddressConversion" ) :
            CREATE_OUSTRING( "com.sun.star.table.CellAddressConversion" );
        rConverter.set( xModelFactory->createInstance( aServiceName ) );
    }
    catch(const Exception& )
    {
    }
    rConverter.setProperty( PROP_XLA1Representation, rAddressString );
    rConverter.setProperty( PROP_ReferenceSheet, nRefSheet );
}

} // namespace

// ============================================================================

ControlConverter::ControlConverter( const Reference< XModel >& rxDocModel,
        const GraphicHelper& rGraphicHelper, bool bDefaultColorBgr ) :
    mxDocModel( rxDocModel ),
    mrGraphicHelper( rGraphicHelper ),
    mbDefaultColorBgr( bDefaultColorBgr )
{
    OSL_ENSURE( mxDocModel.is(), "ControlConverter::ControlConverter - missing document model" );
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

void ControlConverter::convertToMSColor( PropertySet& rPropSet, sal_Int32 nPropId, sal_uInt32& nOleColor, sal_uInt32 nDefault ) const
{
    sal_uInt32 nRGB = 0;
    if (rPropSet.getProperty( nRGB, nPropId ))
        nOleColor = OleHelper::encodeOleColor( nRGB );
    else
        nOleColor = nDefault;
}
void ControlConverter::convertPicture( PropertyMap& rPropMap, const StreamDataSequence& rPicData ) const
{
    if( rPicData.hasElements() )
    {
        OUString aGraphicUrl = mrGraphicHelper.importGraphicObject( rPicData );
        if( !aGraphicUrl.isEmpty() )
            rPropMap.setProperty( PROP_ImageURL, aGraphicUrl );
    }
}

void ControlConverter::convertOrientation( PropertyMap& rPropMap, bool bHorizontal ) const
{
    sal_Int32 nScrollOrient = bHorizontal ? ScrollBarOrientation::HORIZONTAL : ScrollBarOrientation::VERTICAL;
    rPropMap.setProperty( PROP_Orientation, nScrollOrient );
}

void ControlConverter::convertToMSOrientation( PropertySet& rPropSet, bool& bHorizontal ) const
{
    sal_Int32 nScrollOrient = ScrollBarOrientation::HORIZONTAL;
    if ( rPropSet.getProperty( nScrollOrient, PROP_Orientation ) )
        bHorizontal = ( nScrollOrient == ScrollBarOrientation::HORIZONTAL );
}

void ControlConverter::convertVerticalAlign( PropertyMap& rPropMap, sal_Int32 nVerticalAlign ) const
{
    VerticalAlignment eAlign = VerticalAlignment_TOP;
    switch( nVerticalAlign )
    {
        case XML_Top:       eAlign = VerticalAlignment_TOP;     break;
        case XML_Center:    eAlign = VerticalAlignment_MIDDLE;  break;
        case XML_Bottom:    eAlign = VerticalAlignment_BOTTOM;  break;
    }
    rPropMap.setProperty( PROP_VerticalAlign, eAlign );
}

void ControlConverter::convertScrollabilitySettings( PropertyMap& rPropMap,
                                         const AxPairData& rScrollPos, const AxPairData& rScrollArea,
                                         sal_Int32 nScrollBars ) const
{
    Size tmpSize = mrGraphicHelper.convertHmmToAppFont( Size( rScrollArea.first, rScrollArea.second ) );
    Point tmpPos = mrGraphicHelper.convertHmmToAppFont( Point( rScrollPos.first, rScrollPos.second ) );
    rPropMap.setProperty( PROP_ScrollHeight, tmpSize.Height );
    rPropMap.setProperty( PROP_ScrollWidth, tmpSize.Width );
    rPropMap.setProperty( PROP_ScrollTop, tmpPos.Y );
    rPropMap.setProperty( PROP_ScrollLeft, tmpPos.X );
    rPropMap.setProperty( PROP_HScroll, ( nScrollBars & 0x1 ) == 0x1 );
    rPropMap.setProperty( PROP_VScroll, ( nScrollBars & 0x2 ) == 0x2 );
    printf("** imported hscroll %d vscroll %d\n", ( nScrollBars & 0x1 ) == 0x1, ( nScrollBars & 0x2 ) == 0x2);
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

void ControlConverter::bindToSources( const Reference< XControlModel >& rxCtrlModel,
        const OUString& rCtrlSource, const OUString& rRowSource, sal_Int32 nRefSheet ) const
{
    // value binding
    if( !rCtrlSource.isEmpty() ) try
    {
        // first check if the XBindableValue interface is supported
        Reference< XBindableValue > xBindable( rxCtrlModel, UNO_QUERY_THROW );

        // convert address string to cell address struct
        CellAddress aAddress;
        if( !lclExtractAddressFromName( aAddress, mxDocModel, rCtrlSource ) )
        {
            lclPrepareConverter( maAddressConverter, mxDocModel, rCtrlSource, nRefSheet, false );
            if( !maAddressConverter.getProperty( aAddress, PROP_Address ) )
                throw RuntimeException();
        }

        // create argument sequence
        NamedValue aValue;
        aValue.Name = CREATE_OUSTRING( "BoundCell" );
        aValue.Value <<= aAddress;
        Sequence< Any > aArgs( 1 );
        aArgs[ 0 ] <<= aValue;

        // create the CellValueBinding instance and set at the control model
        Reference< XMultiServiceFactory > xModelFactory( mxDocModel, UNO_QUERY_THROW );
        Reference< XValueBinding > xBinding( xModelFactory->createInstanceWithArguments(
            CREATE_OUSTRING( "com.sun.star.table.CellValueBinding" ), aArgs ), UNO_QUERY_THROW );
        xBindable->setValueBinding( xBinding );
    }
    catch(const Exception& )
    {
    }

    // list entry source
    if( !rRowSource.isEmpty() ) try
    {
        // first check if the XListEntrySink interface is supported
        Reference< XListEntrySink > xEntrySink( rxCtrlModel, UNO_QUERY_THROW );

        // convert address string to cell range address struct
        CellRangeAddress aRangeAddr;
        if( !lclExtractRangeFromName( aRangeAddr, mxDocModel, rRowSource ) )
        {
            lclPrepareConverter( maRangeConverter, mxDocModel, rRowSource, nRefSheet, true );
            if( !maRangeConverter.getProperty( aRangeAddr, PROP_Address ) )
                throw RuntimeException();
        }

        // create argument sequence
        NamedValue aValue;
        aValue.Name = CREATE_OUSTRING( "CellRange" );
        aValue.Value <<= aRangeAddr;
        Sequence< Any > aArgs( 1 );
        aArgs[ 0 ] <<= aValue;

        // create the EntrySource instance and set at the control model
        Reference< XMultiServiceFactory > xModelFactory( mxDocModel, UNO_QUERY_THROW );
        Reference< XListEntrySource > xEntrySource( xModelFactory->createInstanceWithArguments(
            CREATE_OUSTRING( "com.sun.star.table.CellRangeListSource"  ), aArgs ), UNO_QUERY_THROW );
        xEntrySink->setListEntrySource( xEntrySource );
    }
    catch(const Exception& )
    {
    }
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

void ControlConverter::convertToAxBorder( PropertySet& rPropSet,
        sal_uInt32& nBorderColor, sal_Int32& nBorderStyle, sal_Int32& nSpecialEffect ) const
{
    sal_Int16 nBorder = API_BORDER_NONE;
    rPropSet.getProperty( nBorder, PROP_Border );
    nBorderStyle = AX_BORDERSTYLE_NONE;
    nSpecialEffect =  AX_SPECIALEFFECT_FLAT;
    switch ( nBorder )
    {
        case API_BORDER_FLAT:
            nBorderStyle = AX_BORDERSTYLE_SINGLE;
            break;
        case API_BORDER_SUNKEN:
            nSpecialEffect =  AX_SPECIALEFFECT_SUNKEN;
        case API_BORDER_NONE:
        default:
            break;
    };
    convertToMSColor( rPropSet, PROP_BorderColor, nBorderColor );
}

void ControlConverter::convertAxVisualEffect( PropertyMap& rPropMap, sal_Int32 nSpecialEffect ) const
{
    sal_Int16 nVisualEffect = (nSpecialEffect == AX_SPECIALEFFECT_FLAT) ? VisualEffect::FLAT : VisualEffect::LOOK3D;
    rPropMap.setProperty( PROP_VisualEffect, nVisualEffect );
}

void ControlConverter::convertToAxVisualEffect( PropertySet& rPropSet, sal_Int32& nSpecialEffect ) const
{
    sal_Int16 nVisualEffect = AX_SPECIALEFFECT_FLAT;
    rPropSet.getProperty( nVisualEffect, PROP_VisualEffect );
    // is this appropriate AX_SPECIALEFFECT_XXXX value ?
    if (nVisualEffect == VisualEffect::LOOK3D )
        nSpecialEffect = AX_SPECIALEFFECT_RAISED;
}

void ControlConverter::convertAxPicture( PropertyMap& rPropMap, const StreamDataSequence& rPicData, sal_uInt32 nPicPos ) const
{
    // the picture
    convertPicture( rPropMap, rPicData );

    // picture position
    sal_Int16 nImagePos = ImagePosition::LeftCenter;
    switch( nPicPos )
    {
        case AX_PICPOS_LEFTTOP:     nImagePos = ImagePosition::LeftTop;     break;
        case AX_PICPOS_LEFTCENTER:  nImagePos = ImagePosition::LeftCenter;  break;
        case AX_PICPOS_LEFTBOTTOM:  nImagePos = ImagePosition::LeftBottom;  break;
        case AX_PICPOS_RIGHTTOP:    nImagePos = ImagePosition::RightTop;    break;
        case AX_PICPOS_RIGHTCENTER: nImagePos = ImagePosition::RightCenter; break;
        case AX_PICPOS_RIGHTBOTTOM: nImagePos = ImagePosition::RightBottom; break;
        case AX_PICPOS_ABOVELEFT:   nImagePos = ImagePosition::AboveLeft;   break;
        case AX_PICPOS_ABOVECENTER: nImagePos = ImagePosition::AboveCenter; break;
        case AX_PICPOS_ABOVERIGHT:  nImagePos = ImagePosition::AboveRight;  break;
        case AX_PICPOS_BELOWLEFT:   nImagePos = ImagePosition::BelowLeft;   break;
        case AX_PICPOS_BELOWCENTER: nImagePos = ImagePosition::BelowCenter; break;
        case AX_PICPOS_BELOWRIGHT:  nImagePos = ImagePosition::BelowRight;  break;
        case AX_PICPOS_CENTER:      nImagePos = ImagePosition::Centered;    break;
        default:    OSL_FAIL( "ControlConverter::convertAxPicture - unknown picture position" );
    }
    rPropMap.setProperty( PROP_ImagePosition, nImagePos );
}

void ControlConverter::convertAxPicture( PropertyMap& rPropMap, const StreamDataSequence& rPicData,
        sal_Int32 nPicSizeMode, sal_Int32 /*nPicAlign*/, bool /*bPicTiling*/ ) const
{
    // the picture
    convertPicture( rPropMap, rPicData );

    // picture scale mode
    sal_Int16 nScaleMode = ImageScaleMode::None;
    switch( nPicSizeMode )
    {
        case AX_PICSIZE_CLIP:       nScaleMode = ImageScaleMode::None;          break;
        case AX_PICSIZE_STRETCH:    nScaleMode = ImageScaleMode::Anisotropic;   break;
        case AX_PICSIZE_ZOOM:       nScaleMode = ImageScaleMode::Isotropic;     break;
        default:    OSL_FAIL( "ControlConverter::convertAxPicture - unknown picture size mode" );
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

void ControlConverter::convertToAxState( PropertySet& rPropSet,
        OUString& rValue, sal_Int32& nMultiSelect, ApiDefaultStateMode eDefStateMode, bool /*bAwtModel*/ ) const
{
    bool bSupportsTriState = eDefStateMode == API_DEFAULTSTATE_TRISTATE;

    sal_Int16 nState = API_STATE_DONTKNOW;

    sal_Bool bTmp = sal_False;
    // need to use State for current state ( I think this is regardless of whether
    // control is awt or not )
    rPropSet.getProperty( nState, PROP_State );

    rValue = rtl::OUString(); // empty e.g. 'don't know'
    if ( nState == API_STATE_UNCHECKED )
        rValue = rtl::OUString( '0' );
    else if ( nState == API_STATE_CHECKED )
        rValue = rtl::OUString( '1' );

    // tristate
    if( bSupportsTriState && rPropSet.getProperty( bTmp, PROP_TriState ) )
        nMultiSelect = AX_SELCTION_MULTI;
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
        default:    OSL_FAIL( "ControlConverter::convertAxOrientation - unknown orientation" );
    }
    convertOrientation( rPropMap, bHorizontal );
}

void ControlConverter::convertToAxOrientation( PropertySet& rPropSet,
        const AxPairData& /*rSize*/, sal_Int32& nOrientation ) const
{
    bool bHorizontal = true;
    convertToMSOrientation( rPropSet, bHorizontal );

    if ( bHorizontal )
        nOrientation = AX_ORIENTATION_HORIZONTAL;
    else
        nOrientation = AX_ORIENTATION_VERTICAL;
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
        case API_CONTROL_RADIOBUTTON:   return CREATE_OUSTRING( "com.sun.star.form.component.RadioButton" );
        case API_CONTROL_EDIT:          return CREATE_OUSTRING( "com.sun.star.awt.UnoControlEditModel" );
        case API_CONTROL_NUMERIC:       return CREATE_OUSTRING( "com.sun.star.awt.UnoControlNumericFieldModel" );
        case API_CONTROL_LISTBOX:       return CREATE_OUSTRING( "com.sun.star.form.component.ListBox" );
        case API_CONTROL_COMBOBOX:      return CREATE_OUSTRING( "com.sun.star.form.component.ComboBox" );
        case API_CONTROL_SPINBUTTON:    return CREATE_OUSTRING( "com.sun.star.form.component.SpinButton" );
        case API_CONTROL_SCROLLBAR:     return CREATE_OUSTRING( "com.sun.star.form.component.ScrollBar" );
        case API_CONTROL_PROGRESSBAR:   return CREATE_OUSTRING( "com.sun.star.awt.UnoControlProgressBarModel" );
        case API_CONTROL_FRAME:         return CREATE_OUSTRING( "com.sun.star.awt.UnoFrameModel" );
        case API_CONTROL_PAGE:          return CREATE_OUSTRING( "com.sun.star.awt.UnoPageModel" );
        case API_CONTROL_MULTIPAGE:     return CREATE_OUSTRING( "com.sun.star.awt.UnoMultiPageModel" );
        case API_CONTROL_DIALOG:        return CREATE_OUSTRING( "com.sun.star.awt.UnoControlDialogModel" );
        default:    OSL_FAIL( "ControlModelBase::getServiceName - no AWT model service supported" );
    }
    else switch( eCtrlType )
    {
        case API_CONTROL_BUTTON:        return CREATE_OUSTRING( "com.sun.star.form.component.CommandButton" );
        case API_CONTROL_FIXEDTEXT:     return CREATE_OUSTRING( "com.sun.star.form.component.FixedText" );
        case API_CONTROL_IMAGE:         return CREATE_OUSTRING( "com.sun.star.form.component.DatabaseImageControl" );
        case API_CONTROL_CHECKBOX:      return CREATE_OUSTRING( "com.sun.star.form.component.CheckBox" );
        case API_CONTROL_RADIOBUTTON:   return CREATE_OUSTRING( "com.sun.star.form.component.RadioButton" );
        case API_CONTROL_EDIT:          return CREATE_OUSTRING( "com.sun.star.form.component.TextField" );
        case API_CONTROL_NUMERIC:       return CREATE_OUSTRING( "com.sun.star.form.component.NumericField" );
        case API_CONTROL_LISTBOX:       return CREATE_OUSTRING( "com.sun.star.form.component.ListBox" );
        case API_CONTROL_COMBOBOX:      return CREATE_OUSTRING( "com.sun.star.form.component.ComboBox" );
        case API_CONTROL_SPINBUTTON:    return CREATE_OUSTRING( "com.sun.star.form.component.SpinButton" );
        case API_CONTROL_SCROLLBAR:     return CREATE_OUSTRING( "com.sun.star.form.component.ScrollBar" );
        case API_CONTROL_GROUPBOX:      return CREATE_OUSTRING( "com.sun.star.form.component.GroupBox" );
        default:    OSL_FAIL( "ControlModelBase::getServiceName - no form component service supported" );
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

void ControlModelBase::convertFromProperties( PropertySet& /*rPropMap*/, const ControlConverter& /*rConv*/ )
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
        case COMCTL_VERSION_50: return mnDataPartId5;
        case COMCTL_VERSION_60: return mnDataPartId6;
    }
    OSL_FAIL( "ComCtlObjectBase::getDataPartId - unxpected version" );
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
    if( mnVersion == COMCTL_VERSION_60 )
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

void AxFontDataModel::exportBinaryModel( BinaryOutputStream& rOutStrm )
{
    maFontData.exportBinaryModel( rOutStrm );
}
void AxFontDataModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    // font name
    if( !maFontData.maFontName.isEmpty() )
        rPropMap.setProperty( PROP_FontName, maFontData.maFontName );

    // font effects
    rPropMap.setProperty( PROP_FontWeight, getFlagValue( maFontData.mnFontEffects, AX_FONTDATA_BOLD, FontWeight::BOLD, FontWeight::NORMAL ) );
    rPropMap.setProperty( PROP_FontSlant, getFlagValue< sal_Int16 >( maFontData.mnFontEffects, AX_FONTDATA_ITALIC, FontSlant_ITALIC, FontSlant_NONE ) );
    rPropMap.setProperty( PROP_FontUnderline, getFlagValue( maFontData.mnFontEffects, AX_FONTDATA_UNDERLINE, maFontData.mbDblUnderline ? FontUnderline::DOUBLE : FontUnderline::SINGLE, FontUnderline::NONE ) );
    rPropMap.setProperty( PROP_FontStrikeout, getFlagValue( maFontData.mnFontEffects, AX_FONTDATA_STRIKEOUT, FontStrikeout::SINGLE, FontStrikeout::NONE ) );
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
        sal_Int32 nAlign = TextAlign::LEFT;
        switch( maFontData.mnHorAlign )
        {
            case AX_FONTDATA_LEFT:      nAlign = TextAlign::LEFT;   break;
            case AX_FONTDATA_RIGHT:     nAlign = TextAlign::RIGHT;  break;
            case AX_FONTDATA_CENTER:    nAlign = TextAlign::CENTER; break;
            default:    OSL_FAIL( "AxFontDataModel::convertProperties - unknown text alignment" );
        }
        // form controls expect short value
        rPropMap.setProperty( PROP_Align, static_cast< sal_Int16 >( nAlign ) );
    }

    // process base class properties
    AxControlModelBase::convertProperties( rPropMap, rConv );
}

void AxFontDataModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& /*rConv */)
{
    rPropSet.getProperty( maFontData.maFontName, PROP_FontName );
    float fontWeight = (float)0;
    if ( rPropSet.getProperty(fontWeight, PROP_FontWeight ) )
        setFlag( maFontData.mnFontEffects, AX_FONTDATA_BOLD, ( fontWeight == FontWeight::BOLD ) );
    sal_Int16 nSlant = FontSlant_NONE;
    if ( rPropSet.getProperty( nSlant, PROP_FontSlant ) )
        setFlag( maFontData.mnFontEffects, AX_FONTDATA_ITALIC, ( nSlant == FontSlant_ITALIC ) );

    sal_Int16 nUnderLine = FontUnderline::NONE;
    if ( rPropSet.getProperty( nUnderLine, PROP_FontUnderline ) )
        setFlag( maFontData.mnFontEffects, AX_FONTDATA_UNDERLINE, nUnderLine != FontUnderline::NONE );
    sal_Int16 nStrikeout = FontStrikeout::NONE ;
    if ( rPropSet.getProperty( nStrikeout, PROP_FontStrikeout ) )
        setFlag( maFontData.mnFontEffects, AX_FONTDATA_STRIKEOUT, nStrikeout !=  FontStrikeout::NONE );

    float fontHeight = 0.0;
    if ( rPropSet.getProperty( fontHeight, PROP_FontHeight ) )
        maFontData.setHeightPoints( static_cast< sal_Int16 >( fontHeight ) );

    // TODO - handle textencoding
    sal_Int16 nAlign = 0;
    if ( rPropSet.getProperty( nAlign, PROP_Align ) )
    {
        switch ( nAlign )
        {
            case TextAlign::LEFT: maFontData.mnHorAlign = AX_FONTDATA_LEFT;   break;
            case TextAlign::RIGHT: maFontData.mnHorAlign = AX_FONTDATA_RIGHT;  break;
            case TextAlign::CENTER: maFontData.mnHorAlign = AX_FONTDATA_CENTER; break;
            default:    OSL_FAIL( "AxFontDataModel::convertFromProperties - unknown text alignment" );
        }
    }
}

// ============================================================================

AxCommandButtonModel::AxCommandButtonModel() :
    mnTextColor( AX_SYSCOLOR_BUTTONTEXT ),
    mnBackColor( AX_SYSCOLOR_BUTTONFACE ),
    mnFlags( AX_CMDBUTTON_DEFFLAGS ),
    mnPicturePos( AX_PICPOS_ABOVECENTER ),
    mnVerticalAlign( XML_Center ),
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


void AxCommandButtonModel::exportBinaryModel( BinaryOutputStream& rOutStrm )
{
    AxBinaryPropertyWriter aWriter( rOutStrm );
    aWriter.writeIntProperty< sal_uInt32 >( mnTextColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnBackColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnFlags );
    aWriter.writeStringProperty( maCaption );
    aWriter.skipProperty(); // pict pos
    aWriter.writePairProperty( maSize );
    aWriter.skipProperty(); // mouse pointer
    aWriter.skipProperty(); // picture data
    aWriter.skipProperty(); // accelerator
    aWriter.writeBoolProperty( mbFocusOnClick ); // binary flag means "do not take focus"
    aWriter.skipProperty(); // mouse icon
    aWriter.finalizeExport();
    AxFontDataModel::exportBinaryModel( rOutStrm );
}

void AxCommandButtonModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x32, 0x05, 0xD7,
        0x69, 0xCE, 0xCD, 0x11, 0xA7, 0x77, 0x00, 0xDD,
        0x01, 0x14, 0x3C, 0x57, 0x22, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6d, 0x73, 0x20,
        0x32, 0x2e, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x6D,
        0x61, 0x6E, 0x64, 0x42, 0x75, 0x74, 0x74, 0x6F,
        0x6E, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
        0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
        0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x16, 0x00,
        0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
        0x43, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x42,
        0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
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
    rConv.convertColor( rPropMap, PROP_TextColor, mnTextColor );
    rConv.convertVerticalAlign( rPropMap, mnVerticalAlign );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    rConv.convertAxPicture( rPropMap, maPictureData, mnPicturePos );
    AxFontDataModel::convertProperties( rPropMap, rConv );
}

void AxCommandButtonModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    rPropSet.getProperty( maCaption, PROP_Label );
    bool bRes = false;
    if ( rPropSet.getProperty( bRes, PROP_Enabled ) )
        setFlag( mnFlags, AX_FLAGS_ENABLED, bRes );
    if ( rPropSet.getProperty( bRes,  PROP_MultiLine ) )
        setFlag( mnFlags, AX_FLAGS_WORDWRAP, bRes );
    rPropSet.getProperty( mbFocusOnClick, PROP_FocusOnClick );

    rConv.convertToMSColor( rPropSet, PROP_TextColor, mnTextColor );
    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor );

    AxFontDataModel::convertFromProperties( rPropSet, rConv );
}
// ============================================================================

AxLabelModel::AxLabelModel() :
    mnTextColor( AX_SYSCOLOR_BUTTONTEXT ),
    mnBackColor( AX_SYSCOLOR_BUTTONFACE ),
    mnFlags( AX_LABEL_DEFFLAGS ),
    mnBorderColor( AX_SYSCOLOR_WINDOWFRAME ),
    mnBorderStyle( AX_BORDERSTYLE_NONE ),
    mnSpecialEffect( AX_SPECIALEFFECT_FLAT ),
    mnVerticalAlign( XML_Top )
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

void AxLabelModel::exportBinaryModel( BinaryOutputStream& rOutStrm )
{
    AxBinaryPropertyWriter aWriter( rOutStrm );
    aWriter.writeIntProperty< sal_uInt32 >( mnTextColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnBackColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnFlags );
    aWriter.writeStringProperty( maCaption );
    aWriter.skipProperty(); // picture position
    aWriter.writePairProperty( maSize );
    aWriter.skipProperty(); // mouse pointer
    aWriter.writeIntProperty< sal_uInt32 >( mnBorderColor );
    aWriter.writeIntProperty< sal_uInt16 >( mnBorderStyle );
    aWriter.writeIntProperty< sal_uInt16 >( mnSpecialEffect );
    aWriter.skipProperty(); // picture
    aWriter.skipProperty(); // accelerator
    aWriter.skipProperty(); // mouse icon
    aWriter.finalizeExport();
    AxFontDataModel::exportBinaryModel( rOutStrm );
}

void AxLabelModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    rPropSet.getProperty( maCaption, PROP_Label );
    bool bRes = false;
    if ( rPropSet.getProperty( bRes, PROP_Enabled ) )
        setFlag( mnFlags, AX_FLAGS_ENABLED, bRes );
    if ( rPropSet.getProperty( bRes,  PROP_MultiLine ) )
        setFlag( mnFlags, AX_FLAGS_WORDWRAP, bRes );

    rConv.convertToMSColor( rPropSet, PROP_TextColor, mnTextColor );
    // VerticleAlign doesn't seem to be read from binary

    // not sure about background color, how do we decide when to set
    // AX_FLAGS_OPAQUE ?
    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor  );
    rConv.convertToAxBorder( rPropSet, mnBorderColor, mnBorderStyle, mnSpecialEffect );

    AxFontDataModel::convertFromProperties( rPropSet, rConv );
}

void AxLabelModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x23, 0x9E, 0x8C, 0x97,
        0xB0, 0xD4, 0xCE, 0x11, 0xBF, 0x2D, 0x00, 0xAA,
        0x00, 0x3F, 0x40, 0xD0, 0x1A, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4C, 0x61, 0x62, 0x65,
        0x6C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
        0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
        0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x0E, 0x00,
        0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
        0x4C, 0x61, 0x62, 0x65, 0x6C, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
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
    rConv.convertColor( rPropMap, PROP_TextColor, mnTextColor );
    rConv.convertVerticalAlign( rPropMap, mnVerticalAlign );
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

void AxImageModel::exportBinaryModel( BinaryOutputStream& rOutStrm )
{
    AxBinaryPropertyWriter aWriter( rOutStrm );
    aWriter.skipProperty(); //undefined
    aWriter.skipProperty(); //undefined
    aWriter.skipProperty(); //auto-size
    aWriter.writeIntProperty< sal_uInt32 >( mnBorderColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnBackColor );
    aWriter.writeIntProperty< sal_uInt8 >( mnBorderStyle );
    aWriter.skipProperty(); // mouse pointer
    aWriter.writeIntProperty< sal_uInt8 >( mnPicSizeMode );
    aWriter.writeIntProperty< sal_uInt8 >( mnSpecialEffect );
    aWriter.writePairProperty( maSize );
    aWriter.skipProperty(); //maPictureData );
    aWriter.writeIntProperty< sal_uInt8 >( mnPicAlign );
    aWriter.writeBoolProperty( mbPicTiling );
    aWriter.writeIntProperty< sal_uInt32 >( mnFlags );
    aWriter.skipProperty(); // mouse icon
    aWriter.finalizeExport();
}

void AxImageModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x92, 0x59, 0x4C,
        0x26, 0x69, 0x1B, 0x10, 0x99, 0x92, 0x00, 0x00,
        0x0B, 0x65, 0xC6, 0xF9, 0x1A, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x49, 0x6D, 0x61, 0x67,
        0x65, 0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D,
        0x62, 0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F,
        0x62, 0x6A, 0x65, 0x63, 0x74, 0x00, 0x0E, 0x00,
        0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E,
        0x49, 0x6D, 0x61, 0x67, 0x65, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
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
    mnListRows( 8 ),
    mnVerticalAlign( XML_Center )
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

void AxMorphDataModelBase::exportBinaryModel( BinaryOutputStream& rOutStrm )
{
    AxBinaryPropertyWriter aWriter( rOutStrm, true );
    aWriter.writeIntProperty< sal_uInt32 >( mnFlags );
    aWriter.writeIntProperty< sal_uInt32 >( mnBackColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnTextColor );
    aWriter.writeIntProperty< sal_Int32 >( mnMaxLength );
    aWriter.writeIntProperty< sal_uInt8 >( mnBorderStyle );
    aWriter.writeIntProperty< sal_uInt8 >( mnScrollBars );
    aWriter.writeIntProperty< sal_uInt8 >( mnDisplayStyle );
    aWriter.skipProperty(); // mouse pointer
    aWriter.writePairProperty( maSize );
    aWriter.writeIntProperty< sal_uInt16 >( mnPasswordChar );
    aWriter.skipProperty(); // list width
    aWriter.skipProperty(); // bound column
    aWriter.skipProperty(); // text column
    aWriter.skipProperty(); // column count
    aWriter.skipProperty(); // mnListRows
    aWriter.skipProperty(); // column info count
    aWriter.skipProperty(); // mnMatchEntry
    aWriter.skipProperty(); // list style
    aWriter.skipProperty(); // mnShowDropButton );
    aWriter.skipProperty();
    aWriter.skipProperty(); // drop down style
    aWriter.writeIntProperty< sal_uInt8 >( mnMultiSelect );
    aWriter.writeStringProperty( maValue );
    aWriter.writeStringProperty( maCaption );
    aWriter.skipProperty(); // mnPicturePos );
    aWriter.writeIntProperty< sal_uInt32 >( mnBorderColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnSpecialEffect );
    aWriter.skipProperty(); // mouse icon
    aWriter.skipProperty(); // maPictureData
    aWriter.skipProperty(); // accelerator
    aWriter.skipProperty(); // undefined
    aWriter.writeBoolProperty(true); // must be 1 for morph
    aWriter.writeStringProperty( maGroupName );
    aWriter.finalizeExport();
    AxFontDataModel::exportBinaryModel( rOutStrm );
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
    mnDisplayStyle = AX_DISPLAYSTYLE_TOGGLE;
}

ApiControlType AxToggleButtonModel::getControlType() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_TOGGLE, "AxToggleButtonModel::getControlType - invalid control type" );
    return API_CONTROL_BUTTON;
}

void AxToggleButtonModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    rPropSet.getProperty( maCaption, PROP_Label );

    bool bRes = false;
    if ( rPropSet.getProperty( bRes,  PROP_MultiLine ) )
        setFlag( mnFlags, AX_FLAGS_WORDWRAP, bRes );

    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor );
    // need to process the image if one exists
    rConv.convertToAxState( rPropSet, maValue, mnMultiSelect, API_DEFAULTSTATE_BOOLEAN, mbAwtModel );
    AxMorphDataModelBase::convertFromProperties( rPropSet, rConv );
}

void AxToggleButtonModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_MultiLine, getFlag( mnFlags, AX_FLAGS_WORDWRAP ) );
    rPropMap.setProperty( PROP_Toggle, true );
    rConv.convertVerticalAlign( rPropMap, mnVerticalAlign );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_NOTSUPPORTED );
    rConv.convertAxPicture( rPropMap, maPictureData, mnPicturePos );
    rConv.convertAxState( rPropMap, maValue, mnMultiSelect, API_DEFAULTSTATE_BOOLEAN, mbAwtModel );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
}

void AxToggleButtonModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
            0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x60, 0x1D, 0xD2, 0x8B,
            0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
            0x00, 0x60, 0x02, 0xF3, 0x21, 0x00, 0x00, 0x00,
            0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
            0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
            0x32, 0x2E, 0x30, 0x20, 0x54, 0x6F, 0x67, 0x67,
            0x6C, 0x65, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E,
            0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D, 0x62,
            0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F, 0x62,
            0x6A, 0x65, 0x63, 0x74, 0x00, 0x15, 0x00, 0x00,
            0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E, 0x54,
            0x6F, 0x67, 0x67, 0x6C, 0x65, 0x42, 0x75, 0x74,
            0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00, 0xF4, 0x39,
            0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
}

// ============================================================================

AxCheckBoxModel::AxCheckBoxModel()
{
    mnDisplayStyle = AX_DISPLAYSTYLE_CHECKBOX;
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
    rConv.convertVerticalAlign( rPropMap, mnVerticalAlign );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxVisualEffect( rPropMap, mnSpecialEffect );
    rConv.convertAxPicture( rPropMap, maPictureData, mnPicturePos );
    rConv.convertAxState( rPropMap, maValue, mnMultiSelect, API_DEFAULTSTATE_TRISTATE, mbAwtModel );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
}

void AxCheckBoxModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    rPropSet.getProperty( maCaption, PROP_Label );

    bool bRes = false;
    if ( rPropSet.getProperty( bRes,  PROP_MultiLine ) )
        setFlag( mnFlags, AX_FLAGS_WORDWRAP, bRes );

    rConv.convertToAxVisualEffect( rPropSet, mnSpecialEffect );
    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor );
    // need to process the image if one exists
    rConv.convertToAxState( rPropSet, maValue, mnMultiSelect, API_DEFAULTSTATE_BOOLEAN, mbAwtModel );
    AxMorphDataModelBase::convertFromProperties( rPropSet, rConv );
}

void AxCheckBoxModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1D, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x43, 0x68, 0x65, 0x63,
        0x6B, 0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00,
        0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65,
        0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74,
        0x00, 0x11, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72,
        0x6D, 0x73, 0x2E, 0x43, 0x68, 0x65, 0x63, 0x6B,
        0x42, 0x6F, 0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
}
// ============================================================================

AxOptionButtonModel::AxOptionButtonModel()
{
    mnDisplayStyle = AX_DISPLAYSTYLE_OPTBUTTON;
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
    rConv.convertVerticalAlign( rPropMap, mnVerticalAlign );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxVisualEffect( rPropMap, mnSpecialEffect );
    rConv.convertAxPicture( rPropMap, maPictureData, mnPicturePos );
    rConv.convertAxState( rPropMap, maValue, mnMultiSelect, API_DEFAULTSTATE_SHORT, mbAwtModel );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
}

void AxOptionButtonModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    rPropSet.getProperty( maCaption, PROP_Label );

    bool bRes = false;
    if ( rPropSet.getProperty( bRes,  PROP_MultiLine ) )
        setFlag( mnFlags, AX_FLAGS_WORDWRAP, bRes );

    rConv.convertToAxVisualEffect( rPropSet, mnSpecialEffect );
    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor );
    // need to process the image if one exists
    rConv.convertToAxState( rPropSet, maValue, mnMultiSelect, API_DEFAULTSTATE_BOOLEAN, mbAwtModel );
    AxMorphDataModelBase::convertFromProperties( rPropSet, rConv );
}

void AxOptionButtonModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x50, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x21, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4F, 0x70, 0x74, 0x69,
        0x6F, 0x6E, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E,
        0x00, 0x10, 0x00, 0x00, 0x00, 0x45, 0x6D, 0x62,
        0x65, 0x64, 0x64, 0x65, 0x64, 0x20, 0x4F, 0x62,
        0x6A, 0x65, 0x63, 0x74, 0x00, 0x15, 0x00, 0x00,
        0x00, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x2E, 0x4F,
        0x70, 0x74, 0x69, 0x6F, 0x6E, 0x42, 0x75, 0x74,
        0x74, 0x6F, 0x6E, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
}

// ============================================================================

AxTextBoxModel::AxTextBoxModel()
{
    mnDisplayStyle = AX_DISPLAYSTYLE_TEXT;
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

void AxTextBoxModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    bool bRes = false;
    if ( rPropSet.getProperty( bRes,  PROP_MultiLine ) )
        setFlag( mnFlags, AX_FLAGS_WORDWRAP, bRes );
    if ( rPropSet.getProperty( bRes,  PROP_HideInactiveSelection ) )
        setFlag( mnFlags, AX_FLAGS_HIDESELECTION, bRes );
    rPropSet.getProperty( maValue, ( mbAwtModel ? PROP_Text : PROP_DefaultText ) );
    if (maValue.isEmpty() && !mbAwtModel)
        // No default value? Then try exporting the current one.
        rPropSet.getProperty( maValue, PROP_Text);
    sal_Int16 nTmp(0);
    if ( rPropSet.getProperty( nTmp, PROP_MaxTextLen ) )
        mnMaxLength = nTmp;
    if ( rPropSet.getProperty( nTmp, PROP_EchoChar ) )
        mnPasswordChar = nTmp;
    if ( rPropSet.getProperty( bRes,  PROP_HScroll ) )
        setFlag( mnScrollBars, AX_SCROLLBAR_HORIZONTAL, bRes );
    if ( rPropSet.getProperty( bRes,  PROP_VScroll ) )
        setFlag( mnScrollBars, AX_SCROLLBAR_VERTICAL, bRes );

    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor, 0x80000005L );

    rConv.convertToAxBorder( rPropSet, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModelBase::convertFromProperties( rPropSet, rConv );
}

void AxTextBoxModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x54, 0x65, 0x78, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x54, 0x65, 0x78, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
}
// ============================================================================

AxNumericFieldModel::AxNumericFieldModel()
{
    mnDisplayStyle = AX_DISPLAYSTYLE_TEXT;
}

ApiControlType AxNumericFieldModel::getControlType() const
{
    OSL_ENSURE( mnDisplayStyle == AX_DISPLAYSTYLE_TEXT, "AxNumericFieldModel::getControlType - invalid control type" );
    return API_CONTROL_NUMERIC;
}

void AxNumericFieldModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_HideInactiveSelection, getFlag( mnFlags, AX_FLAGS_HIDESELECTION ) );
    // TODO: OUString::toDouble() does not handle local decimal separator
    rPropMap.setProperty( mbAwtModel ? PROP_Value : PROP_DefaultValue, maValue.toDouble() );
    rPropMap.setProperty( PROP_Spin, getFlag( mnScrollBars, AX_SCROLLBAR_VERTICAL ) );
    rPropMap.setProperty( PROP_Repeat, true );
    rConv.convertAxBackground( rPropMap, mnBackColor, mnFlags, API_TRANSPARENCY_VOID );
    rConv.convertAxBorder( rPropMap, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModelBase::convertProperties( rPropMap, rConv );
}

void AxNumericFieldModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    bool bRes = false;
    if ( rPropSet.getProperty( bRes,  PROP_HideInactiveSelection ) )
        setFlag( mnFlags, AX_FLAGS_HIDESELECTION, bRes );
    rPropSet.getProperty( maValue, ( mbAwtModel ? PROP_Text : PROP_DefaultText ) );
    if ( rPropSet.getProperty( bRes,  PROP_Spin ) )
        setFlag( mnScrollBars, AX_SCROLLBAR_VERTICAL, bRes );

    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor );

    rConv.convertToAxBorder( rPropSet, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModelBase::convertFromProperties( rPropSet, rConv );
}

void AxNumericFieldModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x54, 0x65, 0x78, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x54, 0x65, 0x78, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
}
// ============================================================================

AxListBoxModel::AxListBoxModel()
{
    mnDisplayStyle = AX_DISPLAYSTYLE_LISTBOX;
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

void AxListBoxModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    bool bRes = false;
    if ( rPropSet.getProperty( bRes, PROP_MultiSelection ) )

    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor );

    rConv.convertToAxBorder( rPropSet, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModelBase::convertFromProperties( rPropSet, rConv );
}

void AxListBoxModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1C, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x4C, 0x69, 0x73, 0x74,
        0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65, 0x64,
        0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72, 0x6D,
        0x73, 0x2E, 0x4C, 0x69, 0x73, 0x74, 0x42, 0x6F,
        0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
}
// ============================================================================

AxComboBoxModel::AxComboBoxModel()
{
    mnDisplayStyle = AX_DISPLAYSTYLE_COMBOBOX;
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

void AxComboBoxModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    // when would we have mnDisplayStyle = AX_DISPLAYSTYLE_DROPDOWN ?
    // #TODO check against msocximex
    mnDisplayStyle = AX_DISPLAYSTYLE_COMBOBOX;
    bool bRes = false;

    if ( rPropSet.getProperty( bRes, PROP_HideInactiveSelection ) )
        setFlag( mnFlags, AX_FLAGS_HIDESELECTION, bRes );
    rPropSet.getProperty( maValue, ( mbAwtModel ? PROP_Text : PROP_DefaultText ) );

    sal_Int16 nTmp(0);
    if ( rPropSet.getProperty( nTmp, PROP_MaxTextLen ) )
        mnMaxLength = nTmp;
    if ( rPropSet.getProperty( bRes, PROP_Autocomplete ) )
    {
        // when to choose AX_MATCHENTRY_FIRSTLETTER ?
        // #TODO check against msocximex
        if ( bRes )
            mnMatchEntry = AX_MATCHENTRY_COMPLETE;
    }
    if ( rPropSet.getProperty( bRes, PROP_Dropdown ) )
    {
        rPropSet.getProperty( mnListRows, PROP_LineCount );
        if ( !mnListRows )
            mnListRows = 1;
    }
    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor );

    rConv.convertToAxBorder( rPropSet, mnBorderColor, mnBorderStyle, mnSpecialEffect );
    AxMorphDataModelBase::convertFromProperties( rPropSet, rConv );
}

void AxComboBoxModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x30, 0x1D, 0xD2, 0x8B,
        0x42, 0xEC, 0xCE, 0x11, 0x9E, 0x0D, 0x00, 0xAA,
        0x00, 0x60, 0x02, 0xF3, 0x1D, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x43, 0x6F, 0x6D, 0x62,
        0x6F, 0x42, 0x6F, 0x78, 0x00, 0x10, 0x00, 0x00,
        0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64, 0x65,
        0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74,
        0x00, 0x11, 0x00, 0x00, 0x00, 0x46, 0x6F, 0x72,
        0x6D, 0x73, 0x2E, 0x43, 0x6F, 0x6D, 0x62, 0x6F,
        0x42, 0x6F, 0x78, 0x2E, 0x31, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
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

void AxSpinButtonModel::exportBinaryModel( BinaryOutputStream& rOutStrm )
{
    AxBinaryPropertyWriter aWriter( rOutStrm );
    aWriter.writeIntProperty< sal_uInt32 >( mnArrowColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnBackColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnFlags );
    aWriter.writePairProperty( maSize );
    aWriter.skipProperty(); // unused
    aWriter.writeIntProperty< sal_Int32 >( mnMin );
    aWriter.writeIntProperty< sal_Int32 >( mnMax );
    aWriter.writeIntProperty< sal_Int32 >( mnPosition );
    aWriter.skipProperty(); // prev enabled
    aWriter.skipProperty(); // next enabled
    aWriter.writeIntProperty< sal_Int32 >( mnSmallChange );
    aWriter.writeIntProperty< sal_Int32 >( mnOrientation );
    aWriter.writeIntProperty< sal_Int32 >( mnDelay );
    aWriter.skipProperty(); // mouse icon
    aWriter.skipProperty(); // mouse pointer

    aWriter.finalizeExport();
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

void AxSpinButtonModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    bool bRes = false;
    if ( rPropSet.getProperty( bRes,  PROP_Enabled ) )
        setFlag( mnFlags, AX_FLAGS_ENABLED, bRes );
    rPropSet.getProperty( mnMin, PROP_SpinValueMin );
    rPropSet.getProperty( mnMax, PROP_SpinValueMax );
    rPropSet.getProperty( mnSmallChange, PROP_SpinIncrement );
    rPropSet.getProperty( mnPosition, ( mbAwtModel ? PROP_SpinValue : PROP_DefaultSpinValue ) );
    rPropSet.getProperty( mnDelay, PROP_RepeatDelay );
    rConv.convertToMSColor( rPropSet, PROP_SymbolColor, mnArrowColor);
    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor );

    rConv.convertToAxOrientation( rPropSet, maSize, mnOrientation );
}

void AxSpinButtonModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] =
    {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0xB0, 0x6F, 0x17, 0x79,
        0xF2, 0xB7, 0xCE, 0x11, 0x97, 0xEF, 0x00, 0xAA,
        0x00, 0x6D, 0x27, 0x76, 0x1F, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x53, 0x70, 0x69, 0x6E,
        0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E, 0x00, 0x10,
        0x00, 0x00, 0x00, 0x45, 0x6D, 0x62, 0x65, 0x64,
        0x64, 0x65, 0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65,
        0x63, 0x74, 0x00, 0x13, 0x00, 0x00, 0x00, 0x46,
        0x6E, 0x42, 0x75, 0x74, 0x74, 0x6F, 0x6E, 0x2E,
        0x31, 0x00, 0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
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

void AxScrollBarModel::exportBinaryModel( BinaryOutputStream& rOutStrm )
{
    AxBinaryPropertyWriter aWriter( rOutStrm );
    aWriter.writeIntProperty< sal_uInt32 >( mnArrowColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnBackColor );
    aWriter.writeIntProperty< sal_uInt32 >( mnFlags );
    aWriter.writePairProperty( maSize );
    aWriter.skipProperty(); // mouse pointer
    aWriter.writeIntProperty< sal_Int32 >( mnMin );
    aWriter.writeIntProperty< sal_Int32 >( mnMax );
    aWriter.writeIntProperty< sal_Int32 >( mnPosition );
    aWriter.skipProperty(); // unused
    aWriter.skipProperty(); // prev enabled
    aWriter.skipProperty(); // next enabled
    aWriter.writeIntProperty< sal_Int32 >( mnSmallChange );
    aWriter.writeIntProperty< sal_Int32 >( mnLargeChange );
    aWriter.writeIntProperty< sal_Int32 >( mnOrientation );
    aWriter.writeIntProperty< sal_Int16 >( mnPropThumb );
    aWriter.writeIntProperty< sal_Int32 >( mnDelay );
    aWriter.skipProperty(); // mouse icon
    aWriter.finalizeExport();
}

void AxScrollBarModel::exportCompObj( BinaryOutputStream& rOutStream )
{
    // should be able to replace this hardcoded foo with
    // proper export info from MS-OLEDS spec.
    static sal_uInt8 const aCompObj[] =
    {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x81, 0xD1, 0xDF,
        0x2F, 0x5E, 0xCE, 0x11, 0xA4, 0x49, 0x00, 0xAA,
        0x00, 0x4A, 0x80, 0x3D, 0x1E, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x73, 0x20,
        0x32, 0x2E, 0x30, 0x20, 0x53, 0x63, 0x72, 0x6F,
        0x6C, 0x6C, 0x42, 0x61, 0x72, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x45, 0x6D, 0x62, 0x65, 0x64, 0x64,
        0x65, 0x64, 0x20, 0x4F, 0x62, 0x6A, 0x65, 0x63,
        0x74, 0x00, 0x12, 0x00, 0x00, 0x00, 0x46, 0x6F,
        0x72, 0x6D, 0x73, 0x2E, 0x53, 0x63, 0x72, 0x6F,
        0x6C, 0x6C, 0x42, 0x61, 0x72, 0x2E, 0x31, 0x00,
        0xF4, 0x39, 0xB2, 0x71, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    rOutStream.writeMemory( aCompObj, sizeof( aCompObj ) );
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

void AxScrollBarModel::convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv )
{
    bool bRes = false;
    if ( rPropSet.getProperty( bRes,  PROP_Enabled ) )
        setFlag( mnFlags, AX_FLAGS_ENABLED, bRes );
    rPropSet.getProperty( mnDelay, PROP_RepeatDelay );
    mnPropThumb = AX_PROPTHUMB_ON; // default
    rConv.convertToMSColor( rPropSet, PROP_SymbolColor, mnArrowColor);
    rConv.convertToMSColor( rPropSet, PROP_BackgroundColor, mnBackColor );
    rConv.convertToAxOrientation( rPropSet, maSize, mnOrientation );

    rPropSet.getProperty( mnMin, PROP_ScrollValueMin );
    rPropSet.getProperty( mnMax, PROP_ScrollValueMax );
    rPropSet.getProperty( mnSmallChange, PROP_LineIncrement );
    rPropSet.getProperty( mnLargeChange, PROP_BlockIncrement );
    rPropSet.getProperty( mnPosition, ( mbAwtModel ? PROP_ScrollValue : PROP_DefaultScrollValue ) );

}

// ============================================================================

AxTabStripModel::AxTabStripModel() :
    AxFontDataModel( false ),   // no support for alignment properties
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
    // adjust for openoffice ( 1 based )
    rPropMap.setProperty( PROP_MultiPageValue, mnSelectedTab + 1);
    rConv.convertColor( rPropMap, PROP_BackgroundColor, mnBackColor );
    AxFontDataModel::convertProperties( rPropMap, rConv );
}

OUString AxTabStripModel::getCaption( sal_Int32 nIndex ) const
{
    return ContainerHelper::getVectorElement( maCaptions, nIndex, OUString() );
}

// ============================================================================

AxContainerModelBase::AxContainerModelBase( bool bFontSupport ) :
    AxFontDataModel( false ),   // no support for alignment properties
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
    return mbAwtModel ? API_CONTROL_FRAME : API_CONTROL_GROUPBOX;
}

void AxFrameModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
    rPropMap.setProperty( PROP_Label, maCaption );
    rPropMap.setProperty( PROP_Enabled, getFlag( mnFlags, AX_CONTAINER_ENABLED ) );
    printf("Frame, converting scroll bits\n");
    rConv.convertScrollabilitySettings( rPropMap, maScrollPos, maLogicalSize, mnScrollBars );
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
    rConv.convertAxPicture( rPropMap, maPictureData, AX_PICPOS_CENTER  );
    printf("UserForm, converting scroll bits\n");
    rConv.convertScrollabilitySettings( rPropMap, maScrollPos, maLogicalSize, mnScrollBars );
    AxContainerModelBase::convertProperties( rPropMap, rConv );
}

HtmlSelectModel::HtmlSelectModel()
{
}

bool
HtmlSelectModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    static String sMultiple( RTL_CONSTASCII_USTRINGPARAM("<SELECT MULTIPLE") );
    static String sSelected( RTL_CONSTASCII_USTRINGPARAM("OPTION SELECTED") );

    OUString sStringContents = rInStrm.readUnicodeArray( rInStrm.size() );

    String data = sStringContents;

    // replace crlf with lf
    data.SearchAndReplaceAll( String( RTL_CONSTASCII_USTRINGPARAM( "\x0D\x0A" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "\x0A" ) ) );
    std::vector< rtl::OUString > listValues;
    std::vector< sal_Int16 > selectedIndices;

    // Ultra hacky parser for the info
    sal_Int32 nTokenCount = comphelper::string::getTokenCount(data, '\n');

    for ( sal_Int32 nToken = 0; nToken < nTokenCount; ++nToken )
    {
        String sLine( data.GetToken( nToken, '\n' ) );
        if ( !nToken ) // first line will tell us if multiselect is enabled
        {
            if ( sLine.CompareTo( sMultiple, sMultiple.Len() ) == COMPARE_EQUAL )
                mnMultiSelect = true;
        }
        // skip first and last lines, no data there
        else if ( nToken < nTokenCount - 1)
        {
            if ( comphelper::string::getTokenCount(sLine, '>') )
            {
                String displayValue  = sLine.GetToken( 1, '>' );
                if ( displayValue.Len() )
                {
                    // Really we should be using a proper html parser
                    // escaping some common bits to be escaped
                    displayValue.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "&lt;" ) ), String( RTL_CONSTASCII_USTRINGPARAM("<") ) );
                    displayValue.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "&gt;" ) ), String( RTL_CONSTASCII_USTRINGPARAM(">") ) );
                    displayValue.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "&quot;" ) ), String( RTL_CONSTASCII_USTRINGPARAM("\"") ) );
                    displayValue.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "&amp;" ) ), String( RTL_CONSTASCII_USTRINGPARAM("&") ) );
                    listValues.push_back( displayValue );
                    if( sLine.Search( sSelected ) != STRING_NOTFOUND )
                        selectedIndices.push_back( static_cast< sal_Int16 >( listValues.size() ) - 1 );
                }
            }
        }
    }
    if ( !listValues.empty() )
    {
        msListData.realloc( listValues.size() );
        sal_Int32 index = 0;
        for( std::vector< rtl::OUString >::iterator it = listValues.begin(); it != listValues.end(); ++it, ++index )
             msListData[ index ] = *it;
    }
    if ( !selectedIndices.empty() )
    {
        msIndices.realloc( selectedIndices.size() );
        sal_Int32 index = 0;
        for( std::vector< sal_Int16 >::iterator it = selectedIndices.begin(); it != selectedIndices.end(); ++it, ++index )
             msIndices[ index ] = *it;
    }
    return sal_True;
}


void
HtmlSelectModel::convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const
{
   rPropMap.setProperty( PROP_StringItemList, msListData );
   rPropMap.setProperty( PROP_SelectedItems, msIndices );
   rPropMap.setProperty( PROP_Dropdown, true );
   AxListBoxModel::convertProperties( rPropMap, rConv );
}

HtmlTextBoxModel::HtmlTextBoxModel()
{
}

bool
HtmlTextBoxModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    OUString sStringContents = rInStrm.readUnicodeArray( rInStrm.size() );
#ifdef DEBUG
    // in msocximex ( where this is ported from, it appears *nothing* is read
    // from the control stream ), surely there is some useful info there ?
    OSL_TRACE("HtmlTextBoxModel::importBinaryModel - string contents of stream :");
    OSL_TRACE("%s", rtl::OUStringToOString( sStringContents, RTL_TEXTENCODING_UTF8 ).getStr() );
#else
    (void) rInStrm;
#endif
    return true;
}
// ============================================================================

EmbeddedControl::EmbeddedControl( const OUString& rName ) :
    maName( rName )
{
}

EmbeddedControl::~EmbeddedControl()
{
}

ControlModelBase* EmbeddedControl::createModelFromGuid( const OUString& rClassId )
{
    OUString aClassId = rClassId;//.toAsciiUpperCase();

    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_COMMANDBUTTON ) )     return &createModel< AxCommandButtonModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_LABEL ) )             return &createModel< AxLabelModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_IMAGE ) )             return &createModel< AxImageModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_TOGGLEBUTTON ) )      return &createModel< AxToggleButtonModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_CHECKBOX ) )          return &createModel< AxCheckBoxModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_OPTIONBUTTON ) )      return &createModel< AxOptionButtonModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_TEXTBOX ) )           return &createModel< AxTextBoxModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_LISTBOX ) )           return &createModel< AxListBoxModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_COMBOBOX ) )          return &createModel< AxComboBoxModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_SPINBUTTON ) )        return &createModel< AxSpinButtonModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_SCROLLBAR ) )         return &createModel< AxScrollBarModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( AX_GUID_FRAME ) )             return &createModel< AxFrameModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( COMCTL_GUID_SCROLLBAR_60 ) )  return &createModel< ComCtlScrollBarModel >( COMCTL_VERSION_60 );
    if( aClassId.equalsIgnoreAsciiCaseAscii( HTML_GUID_SELECT ) )  return &createModel< HtmlSelectModel >();
    if( aClassId.equalsIgnoreAsciiCaseAscii( HTML_GUID_TEXTBOX ) ) return &createModel< HtmlTextBoxModel >();

    mxModel.reset();
    return 0;
}

OUString EmbeddedControl::getServiceName() const
{
    return mxModel.get() ? mxModel->getServiceName() : OUString();
}

bool EmbeddedControl::convertProperties( const Reference< XControlModel >& rxCtrlModel, const ControlConverter& rConv ) const
{
    if( mxModel.get() && rxCtrlModel.is() && !maName.isEmpty() )
    {
        PropertyMap aPropMap;
        aPropMap.setProperty( PROP_Name, maName );
        try
        {
            aPropMap.setProperty( PROP_GenerateVbaEvents, true);
        }
        catch(const Exception& )
        {
        }
        mxModel->convertProperties( aPropMap, rConv );
        PropertySet aPropSet( rxCtrlModel );
        aPropSet.setProperties( aPropMap );
        return true;
    }
    return false;
}

bool EmbeddedControl::convertFromProperties( const Reference< XControlModel >& rxCtrlModel, const ControlConverter& rConv )
{
    if( mxModel.get() && rxCtrlModel.is() && !maName.isEmpty() )
    {
        PropertySet aPropSet( rxCtrlModel );
        aPropSet.getProperty( maName, PROP_Name );
        mxModel->convertFromProperties( aPropSet, rConv );
        return true;
    }
    return false;
}

// ============================================================================

EmbeddedForm::EmbeddedForm( const Reference< XModel >& rxDocModel,
        const Reference< XDrawPage >& rxDrawPage, const GraphicHelper& rGraphicHelper, bool bDefaultColorBgr ) :
    maControlConv( rxDocModel, rGraphicHelper, bDefaultColorBgr ),
    mxModelFactory( rxDocModel, UNO_QUERY ),
    mxFormsSupp( rxDrawPage, UNO_QUERY )
{
    OSL_ENSURE( mxModelFactory.is(), "EmbeddedForm::EmbeddedForm - missing service factory" );
}

Reference< XControlModel > EmbeddedForm::convertAndInsert( const EmbeddedControl& rControl, sal_Int32& rnCtrlIndex )
{
    if( mxModelFactory.is() && rControl.hasModel() ) try
    {
        // create the UNO control model
        OUString aServiceName = rControl.getServiceName();
        Reference< XFormComponent > xFormComp( mxModelFactory->createInstance( aServiceName ), UNO_QUERY_THROW );
        Reference< XControlModel > xCtrlModel( xFormComp, UNO_QUERY_THROW );

        // insert the control into the form
        Reference< XIndexContainer > xFormIC( createXForm(), UNO_SET_THROW );
        rnCtrlIndex = xFormIC->getCount();
        xFormIC->insertByIndex( rnCtrlIndex, Any( xFormComp ) );

        // convert the control properties
        if( rControl.convertProperties( xCtrlModel, maControlConv ) )
            return xCtrlModel;
    }
    catch(const Exception& )
    {
    }
    return Reference< XControlModel >();
}

Reference< XIndexContainer > EmbeddedForm::createXForm()
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
        catch(const Exception& )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
